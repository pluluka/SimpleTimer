/*
 * ExtButton.h
 *
 *  Created on: Mar 1, 2023
 *      Author: tochk
 *      Реализация кнопок с механизмом антидребезга
 */

#ifndef EXTBUTTON_EXTBUTTON_H_
#define EXTBUTTON_EXTBUTTON_H_



/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include <stddef.h>


/* Данные алгоритма антидребезга.
 * Эту структуру необходимо обьявлять(пустую) для каждой кнопки,
 * и вносить указатель в структуру t_ExtButton при инициализации*/
typedef struct{
	uint8_t			cnt;
	uint8_t			prev_state;
} t_BtnAntiBouncing;

/* EXT_BTN_TYPES */
#define EXT_BTN_TYPE_RISING		0x01	// фиксация перехода с низкого уровня на высокий
#define EXT_BTN_TYPE_FALLING	0x02	// фиксация перехода с высокого уровня на низкий
/* используйте (EXT_BTN_TYPE_RISING | EXT_BTN_TYPE_FALING) - для обоих типов */

/* Кнопка */
typedef struct{
	GPIO_TypeDef*	gpio;			// GPIO
	uint32_t		pin;			// pin
	uint8_t			type;			// Тип обработки перепада уровня на порту кнопки, см EXT_BTN_TYPES
	uint8_t			pressed;		// Зафиксирован перепад типа type (с учетом антидребезга),
									// после обработки, необходимо сбрасывать в '0' вручную
	uint8_t			state;			// Текущее состояние [0:1]
	void* 			(*BtnInitFunc)(void);		// указатель на функцию инициализации GPIO
	void* 			(*BthHandlerFunc)(void);	// указатель на функцию-обработчик (если есть такая необходимость)
												// данная ф-я будет вызвана в обработчике прерывания таймера
	t_BtnAntiBouncing*	antiBouncing;	// указатель на структуру с данными алгоритма антидребезга
} t_ExtButton;


void ExtButton_Init(void);
void ExtButton_Add(t_ExtButton* extBtn);


#endif /* EXTBUTTON_EXTBUTTON_H_ */
