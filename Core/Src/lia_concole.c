#include "lia_console.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static bool s_echo_enabled = true;

void LiaConsole_SetEcho(bool enable)
{
    s_echo_enabled = enable;
}

void LiaConsole_Print(const char* s)
{
    if (!s) return;
    HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

void LiaConsole_Printf(const char* fmt,...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    LiaConsole_Print(buf);
}

static inline void echo_char(uint8_t ch)
{
    if (!s_echo_enabled) return;
    (void)HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY);
}

/* 退格回显： "\b \b" 以覆盖字符，兼容多数终端 */
static inline void echo_backspace(void)
{
    if (!s_echo_enabled) return;
    const uint8_t seq[3] = { '\b', ' ', '\b' };
    (void)HAL_UART_Transmit(&huart2, (uint8_t*)seq, sizeof(seq), HAL_MAX_DELAY);
}

int LiaConsole_ReadLine(char* buf, uint32_t maxlen)
{
    if (!buf || maxlen == 0) return -1;

    uint32_t idx = 0;

    while (1) {
        uint8_t ch = 0;
        if (HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY) != HAL_OK) {
            return -1;
        }

        /* 行结束：兼容 CR/LF */
        if (ch == '\r' || ch == '\n') {
            if (idx == 0) {
                /* 忽略空行，继续等待 */
                continue;
            }
            break;
        }

        /* 退格处理：Backspace(0x08) 或 DEL(0x7F) */
        if (ch == 0x08u || ch == 0x7Fu) {
            if (idx > 0) {
                idx--;
                echo_backspace();
            }
            continue;
        }

        /* 常规字符：写入缓冲并回显 */
        if (idx < maxlen - 1) {
            buf[idx++] = (char)ch;
            echo_char(ch);
        }
        /* 如果超长则丢弃额外输入，直到遇到行结束符。 */
    }

    buf[idx] = '\0';
    return (int)idx;
}

int LiaConsole_ReadFloat(const char* prompt, float* out_val)
{
    if (!out_val) return -1;

    char line[32];

    if (prompt) {
        LiaConsole_Print(prompt);
    }
    LiaConsole_Print("\r\n> ");

    int n = LiaConsole_ReadLine(line, sizeof(line));
    if (n <= 0) return -1;

    char* endp = NULL;
    float val = strtof(line, &endp);

    /* 检查是否成功解析（至少消费了一个字符） */
    if (endp == line) {
        return -2; /* 格式错误 */
    }

    *out_val = val;
    return 0;
}
