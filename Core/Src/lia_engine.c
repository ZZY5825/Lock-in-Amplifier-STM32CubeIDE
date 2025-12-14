#include "lia_engine.h"
#include <math.h>
#include <string.h>

/* ===================== 内部常量与状态 ===================== */

/* 系统固定延时（秒），相位补偿使用：phi = -2*pi*f*delay */
#ifndef LIA_SYS_DELAY_SEC
#define LIA_SYS_DELAY_SEC  0.002777778f
#endif

/* 幅度标度：-1..1 → 码值 → 输入 Vpp 的近似换算
   与旧逻辑保持相近：阻抗与直流偏置影响通过校准项给出。 */
static const float s_amp_offset_ratio = 1.0f;

/* DAC 映射的比例（与 2047/2048 的近似一致） */
static const float s_dac_fs_frac = (2047.0f / 2048.0f);

/* 最近一次结果（便于 UI 查询） */
static LiaLastReport s_last = {0.0f, 0.0f};

/* 解调用 NCO（与参考输出一致频率） */
static LiaNCO s_nco_demod;

/* ADC 采样器句柄 */
static ADC_DMA_Sampler_t s_adc_sampler;

/* 核心状态 */
static LiaEngineState s;

/* 当前参考频率（从串口读取，含边界钳制） */
static float s_f_ref = 1000.0f;

/* ===================== 数据驱动的校准段定义 ===================== */

/* 分段线性校准：gain = k*f + b，phase_deg = raw + (k*f + b) */
typedef struct {
    float f_min;
    float f_max;
    float k;
    float b;
} SegCal;

/* 幅度粗校准段（基于原始经验参数，覆盖 10 Hz - 100 kHz） */
static const SegCal s_amp_coarse[] = {
    /* Zone1: 10..50 */
    {10.0f,    50.0f,    -0.00440f, 1.3074f},
    /* Zone2: 50..100 */
    {50.0f,    100.0f,   -0.000152f, 1.0950f},
    /* Zone3: 100..500 */
    {100.0f,   500.0f,   -5.00e-7f,  1.0805f},
    /* Zone4: 500..2000 */
    {500.0f,   2000.0f,  -1.13e-6f,  1.0808f},
    /* Zone5: 2000..10000 */
    {2000.0f,  10000.0f,  5.00e-7f,  1.0776f},
    /* Zone6: 10000..30000 */
    {10000.0f, 30000.0f,  2.00e-7f,  1.0781f},
    /* Zone7: 30000..60000 */
    {30000.0f, 60000.0f,  2.00e-7f,  1.0781f},
    /* Zone8: 60000..80000 */
    {60000.0f, 80000.0f, -1.50e-7f,  1.0802f},
    /* Zone9: 80000..100000+ */
    {80000.0f, 100000.0f,-1.25e-6f,  1.1680f}
};

/* 相位粗校准段（度），phase_corr = k*f + b，然后归一化到 [-180, 180) */
static const SegCal s_phase_coarse[] = {
    {10.0f,    50.0f,    0.8873f,  -59.17f},
    {50.0f,    100.0f,   0.0312f,  -16.37f},
    {100.0f,   500.0f,  -0.0657f,   -6.68f},
    {500.0f,   2000.0f, -0.0763f,   -1.37f},
    {2000.0f,  10000.0f,-0.0768f,   -0.37f},
    {10000.0f, 30000.0f,-0.0768f,   -0.27f},
    {30000.0f, 60000.0f,-0.0767f,   -1.97f},
    {60000.0f, 80000.0f,-0.0767f,   -2.39f},
    {80000.0f, 100000.0f,-0.0767f,  -2.31f}
};

/* 细校准（幅度）：低频与高频残差，用线性插值表达为一个“乘性修正” */
static float amp_fine_factor(float f)
{
    /* 低频 10..50 Hz：从 ~0.936 逐步上升至 ~1.003（参考你的注释） */
    if (f <= 30.0f) {
        /* 25 Hz→0.936，斜率估计：~0.0024/Hz 相对 25 Hz */
        return 0.936f + 0.0024f * (f - 25.0f);
    } else if (f <= 50.0f) {
        /* 35 Hz→0.960 到 48 Hz→1.003，斜率 ~0.0033/Hz */
        return 0.960f + 0.0033f * (f - 35.0f);
    }
    /* 高频 70 k..100 k：略微抬升（82 k→1.0134，98 k→1.0273） */
    else if (f >= 70000.0f) {
        return 1.0134f + 8.68e-7f * (f - 82000.0f);
    }
    return 1.0f;
}

/* 细校准（相位，单位度）：低频加正值，高频整体减常数 */
static float phase_fine_offset_deg(float f)
{
    if (f <= 50.0f) {
        /* 25 Hz 加 +12.33，48 Hz 加 +1.33，斜率 -0.478/Hz */
        return 12.33f + (f - 25.0f) * -0.478f;
    } else if (f >= 40000.0f) {
        return -2.3f;
    }
    return 0.0f;
}

/* ===================== 内部工具函数 ===================== */

/* 选段并计算 k*f + b；若不在任何段内，使用边界段钳制 */
static float seg_eval(const SegCal* segs, size_t count, float f)
{
    if (count == 0) return 1.0f;
    for (size_t i = 0; i < count; ++i) {
        if (f >= segs[i].f_min && f <= segs[i].f_max) {
            return segs[i].k * f + segs[i].b;
        }
    }
    /* 越界处理：小于最小频率用第一个段在 f_min 的值，大于最大用最后一段在 f_max 的值 */
    if (f < segs[0].f_min) {
        return segs[0].k * segs[0].f_min + segs[0].b;
    }
    return segs[count - 1].k * segs[count - 1].f_max + segs[count - 1].b;
}

/* 归一化相位到 [-180, 180) */
static float norm_phase_deg(float deg)
{
    while (deg >= 180.0f) deg -= 360.0f;
    while (deg <  -180.0f) deg += 360.0f;
    return deg;
}

/* 计算一阶低通系数：alpha = 1 - exp(-2π BW / F_block) ，并钳制到 [0,1] */
static float compute_alpha(float block_fs, float bw_hz)
{
    float a = 1.0f - expf(-2.0f * (float)M_PI * bw_hz / block_fs);
    if (a < 0.0f) a = 0.0f;
    if (a > 1.0f) a = 1.0f;
    return a;
}

/* 按当前 BW 档位更新 alpha */
static void update_alpha(void)
{
    float bw = LIA_BW_TABLE[s.bw_index];
    s.alpha = compute_alpha(s.Fs_block, bw);
}

/* 单块处理：完成抽取累积、块平均与单极点低通 */
static void process_block(uint16_t* p, uint32_t len)
{
    for (uint32_t n = 0; n < len; ++n) {
        /* 12-bit ADC → [-1, 1] */
        float x = ((float)p[n] - 2048.0f) * (1.0f / 2048.0f);

        /* NCO 步进（解调 IQ） */
        float s_q, s_i;
        LIA_NCO_Step(&s_nco_demod, &s_q, &s_i);  /* s_q=sin, s_i=cos */

        s.i_sum += x * s_i;
        s.q_sum += x * s_q;
        s.decim_counter++;

        if (s.decim_counter >= LIA_DECIM_FACTOR) {
            float invN = 1.0f / (float)LIA_DECIM_FACTOR;
            float i_block = s.i_sum * invN;
            float q_block = s.q_sum * invN;

            /* 单极点低通：y ← y + α*(x - y) */
            s.I_lp += s.alpha * (i_block - s.I_lp);
            s.Q_lp += s.alpha * (q_block - s.Q_lp);

            s.decim_counter = 0;
            s.i_sum = 0.0f;
            s.q_sum = 0.0f;

            s.print_counter++;
        }
    }
}

/* 计算并上报幅度与相位（包含粗校准 + 细校准） */
static void compute_and_report(void)
{
    float I = s.I_lp;
    float Q = s.Q_lp;

    /* 原始幅度（归一化）：sqrt(I^2 + Q^2) */
    float A_norm = sqrtf(I * I + Q * Q);

    /* 近似输入 Vpp（未校准）：scale → 电压 → 除以前端增益 */
    float k_vpp = (LIA_ADC_VREF * 2.0f) / LIA_GAIN_TABLE[g_lia_gain_index];
    float Vpp = (A_norm / s_amp_offset_ratio) * k_vpp;

    /* 粗校准（乘性）与细校准（乘性） */
    float gain_coarse = seg_eval(s_amp_coarse, sizeof(s_amp_coarse) / sizeof(s_amp_coarse[0]), s_f_ref);
    float gain_fine   = amp_fine_factor(s_f_ref);
    Vpp *= gain_coarse * gain_fine;

    /* 原始相位：与原逻辑同符号（取负），转为度 */
    float phase_deg = -atan2f(Q, I) * (180.0f / (float)M_PI);

    /* 粗校准（加性）与细校准（加性） */
    float phase_coarse = seg_eval(s_phase_coarse, sizeof(s_phase_coarse) / sizeof(s_phase_coarse[0]), s_f_ref);
    float phase_fine   = phase_fine_offset_deg(s_f_ref);
    phase_deg = norm_phase_deg(phase_deg + phase_coarse + phase_fine);

    s_last.Vpp      = Vpp;
    s_last.PhaseDeg = phase_deg;

    LiaConsole_Printf("Vpp=%.6f, Phase=%.2f deg\r\n", Vpp, phase_deg);
}

/* ===================== 对外接口实现 ===================== */

void LiaEngine_SetRefFreq(float f_ref_hz)
{
    /* 钳制到 10..100000 Hz */
    if (f_ref_hz < 10.0f)      f_ref_hz = 10.0f;
    if (f_ref_hz > 100000.0f)  f_ref_hz = 100000.0f;

    s_f_ref = f_ref_hz;

    /* 相位校准：phi0 = -2*pi*f*delay */
    float phi0 = -2.0f * (float)M_PI * s_f_ref * LIA_SYS_DELAY_SEC;
    LIA_NCO_SetFreq(&s_nco_demod, s_f_ref);
    LIA_NCO_SetPhaseOffset(&s_nco_demod, phi0);

    /* 参考信号输出（DAC）也保持一致频率 */
    LiaRefGen_SetFreq(s_f_ref);
}

void LiaEngine_Init(void)
{
    /* 1) 按键模块 */
    Buttons_Init();

    /* 2) 获取 f_ref（阻塞读取） */
    float f_in = 0.0f;
    if (LiaConsole_ReadFloat("f_ref (Hz)", &f_in) != 0) {
        f_in = 1000.0f;
    }
    LiaEngine_SetRefFreq(f_in);

    /* 3) 初始化状态 */
    memset(&s, 0, sizeof(s));
    s.bw_index  = LIA_DEFAULT_BW_INDEX;
    s.Fs_block  = LIA_FS_HZ / (float)LIA_DECIM_FACTOR;
    s.I_lp      = 0.0f;
    s.Q_lp      = 0.0f;
    update_alpha();

    /* 4) 初始化解调 NCO（采样率 LIA_FS_HZ，包含系统延迟相位补偿） */
    float phi0 = -2.0f * (float)M_PI * s_f_ref * LIA_SYS_DELAY_SEC;
    LIA_NCO_Init(&s_nco_demod, LIA_FS_HZ, s_f_ref, phi0);

    /* 5) 初始化参考输出（DAC+NCO），并启动（含 TIM2） */
    LiaRefGen_Init(s_f_ref, -1.0f, -1.0f);     /* 使用默认幅度/偏置 */
    LiaRefGen_StartWithTIM2();

    /* 6) 初始化并启动 ADC 采样器（与 hadc1 绑定） */
    ADC_DMA_Sampler_Init(&s_adc_sampler, &hadc1);
    if (ADC_DMA_Sampler_Start(&s_adc_sampler) != HAL_OK) {
        LiaConsole_Print("ADC DMA start failed.\r\n");
    }

    LiaConsole_Printf("LIA engine init. f_ref=%.3f Hz, BW=%.3f Hz\r\n",
                      s_f_ref, LIA_BW_TABLE[s.bw_index]);
}

void LiaEngine_Run(void)
{
    /* 1) 处理 ADC 半缓冲块 */
    if (ADC_DMA_Sampler_IsHalfReady()) {
        ADC_DMA_Sampler_ClearHalfFlag();
        uint16_t* buf = ADC_DMA_Sampler_GetBuffer();
        process_block(&buf[0], ADC_BLOCK_SIZE);
    }
    /* 2) 处理 ADC 满缓冲块 */
    if (ADC_DMA_Sampler_IsFullReady()) {
        ADC_DMA_Sampler_ClearFullFlag();
        uint16_t* buf = ADC_DMA_Sampler_GetBuffer();
        process_block(&buf[ADC_BLOCK_SIZE], ADC_BLOCK_SIZE);
    }

    /* 3) 处理 DAC 半缓冲补填 */
    LiaRefGen_Service();

    /* 4) 按键去抖与事件响应 */
    Buttons_Service();

    /* 带宽 + */
    if (lia_evt_bw_up) {
        lia_evt_bw_up = 0;
        if (s.bw_index < (LIA_BW_MODE_COUNT - 1)) {
            s.bw_index++;
            update_alpha();
            LiaConsole_Printf("BW -> %.3f Hz\r\n", LIA_BW_TABLE[s.bw_index]);
        }
    }
    /* 带宽 - */
    if (lia_evt_bw_down) {
        lia_evt_bw_down = 0;
        if (s.bw_index > 0) {
            s.bw_index--;
            update_alpha();
            LiaConsole_Printf("BW -> %.3f Hz\r\n", LIA_BW_TABLE[s.bw_index]);
        }
    }
    /* 增益档位事件 */
    if (lia_evt_gain_step) {
        lia_evt_gain_step = 0;
        g_lia_gain_index++;
        if (g_lia_gain_index >= LIA_GAIN_MODE_COUNT) {
            g_lia_gain_index = 0;
        }
        LiaConsole_Printf("GAIN -> %.3f\r\n", LIA_GAIN_TABLE[g_lia_gain_index]);
    }
    /* 如需复位事件，可在此处理：
       if (lia_evt_reset) { lia_evt_reset = 0; NVIC_SystemReset(); }
    */

    /* 5) 周期性打印 */
    if (s.print_counter >= LIA_PRINT_PERIOD_BLOCKS) {
        s.print_counter = 0;
        compute_and_report();
    }
}

int LiaEngine_GetLastReport(LiaLastReport* r)
{
    if (!r) return -1;
    *r = s_last;
    return 0;
}




