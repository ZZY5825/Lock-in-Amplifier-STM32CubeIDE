/* buttons.c */
#include <lia_settings.h>
#include "buttons.h"
#include "main.h"

static DebounceButton btn_plus;
static DebounceButton btn_minus;
static DebounceButton btn_gain;
//static DebounceButton btn_reset;

extern volatile uint8_t g_bw_plus_event;
extern volatile uint8_t g_bw_minus_event;
extern volatile uint8_t g_gain_event;
//extern volatile uint8_t g_reset_event;

void Buttons_Init(void)
{
    Debounce_Init(&btn_plus,  BW_PLUS_GPIO_Port,  BW_PLUS_Pin);
    Debounce_Init(&btn_minus, BW_MINUS_GPIO_Port, BW_MINUS_Pin);
    Debounce_Init(&btn_gain,  GAIN_GPIO_Port,    GAIN_Pin);

    g_bw_plus_event  = 0;
    g_bw_minus_event = 0;
    g_gain_event     = 0;
}


void Buttons_OnExti(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BW_PLUS_Pin) {
        Debounce_OnExtiIRQ(&btn_plus);
    } else if (GPIO_Pin == BW_MINUS_Pin) {
        Debounce_OnExtiIRQ(&btn_minus);
    } else if (GPIO_Pin == GAIN_Pin) {
        Debounce_OnExtiIRQ(&btn_gain);
    }

}

void Buttons_Service(void)
{
    if (Debounce_PollPressed(&btn_plus, 1u)) {
        g_bw_plus_event = 1u;
    }

    if (Debounce_PollPressed(&btn_minus, 1u)) {
        g_bw_minus_event = 1u;
    }

    if (Debounce_PollPressed(&btn_gain, 1u)) {
        g_gain_event = 1u;
    }

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    Buttons_OnExti(GPIO_Pin);
}
