/*
 * HS420561.h
 *
 *  Created on: Feb 6, 2023
 *      Author: tochk
 *      Драйвер 4-х сегментного индикатора, на базе сдвигового регистра
 */

#ifndef HS420561_BY_HC595_HS420561_H_
#define HS420561_BY_HC595_HS420561_H_

#include "stm32f1xx.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include "N74HC595.h"


typedef enum{
	NORMAL,			// непрерывный
	FLASHING,		// мигающий
	CIRCLE
} t_HS420561_Segment_mode;

typedef enum{
	POINT_OFF,
	POINT_ON
} t_HS420561_Point_mode;

/* Параметры сегмента */
typedef struct{
	uint8_t 				val;	// отображаемое значение
	t_HS420561_Segment_mode mode;	// режим отображения
	t_HS420561_Point_mode	point;	// вкл/откл точку 0-выкл, 1-вкл
} t_HS420561_Segment;

/* Индикатор */
typedef struct{
	t_HS420561_Segment S1;	// сегмент 1
	t_HS420561_Segment S2;	// сегмент 2
	t_HS420561_Segment S3;	// сегмент 3
	t_HS420561_Segment S4;	// сегмент 4
} t_HS420561_Base;



void HS420561_Init(t_HS420561_Base* base);


#endif /* HS420561_BY_HC595_HS420561_H_ */
