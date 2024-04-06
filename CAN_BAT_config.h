
#pragma once

#include "stdint.h"

#define USE_SSD1306

#define STM32F1
//#define STM32F4
//#define STM32H7
//#define STM32F7

#define LED_RED(PinState) 	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,PinState);
#define LED_GREEN 	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
#define LED_BLUE 	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_15);

#define CANBUS_PORT		hcan1
#define TIM_PORT		htim7

#define SELF_ID 0x80EA38F0 //0x15353316 (CAN BUS Trasmitter ID)

#define TIM_SEND


#if defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#else
#error "MAL"
#endif
