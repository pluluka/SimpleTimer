/*
 * zummer.h
 *
 * Генератор для пищалки(зуммера, пьезо и т.п.)
 * На базе таймера в режиме PWM
 * мин частота 1200Гц
 *
 *  Created on: 8 мар. 2023 г.
 *      Author: tochk
 */

#ifndef ZUMMER_ZUMMER_H_
#define ZUMMER_ZUMMER_H_


#include "stm32f1xx.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"


typedef struct{
	uint16_t	freq;		// частота (мин 1200)
	uint16_t	pulseTime;	// время пульсации (мс)
} t_ZummerBase;



void ZummerInit(t_ZummerBase* zummer);
void ZummerON(void);
void ZummerOFF(void);


#endif /* ZUMMER_ZUMMER_H_ */
