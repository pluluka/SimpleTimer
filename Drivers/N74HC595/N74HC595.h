/*
 * N74HC595.h
 *
 *  Created on: 4 февр. 2023 г.
 *      Author: tochk
 */

#ifndef N74HC595_N74HC595_H_
#define N74HC595_N74HC595_H_


#include "stm32f1xx.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"


#define	N74HC595_SPI				SPI2
#define N74HC595_GPIO_STCP			GPIOB
#define N74HC595_GPIO_STCP_PIN		LL_GPIO_PIN_12



void N74HC595_Init(void);
void N74HC595_Write(uint8_t data);


#endif /* N74HC595_N74HC595_H_ */
