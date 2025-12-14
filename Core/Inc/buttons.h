/* buttons.h */
#ifndef BUTTONS_H
#define BUTTONS_H

#include "debounce.h"

void Buttons_Init(void);

void Buttons_OnExti(uint16_t GPIO_Pin);

void Buttons_Service(void);

#endif /* BUTTONS_H */
