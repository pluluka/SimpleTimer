/*
 * HS420561.c
 *
 *  Created on: Feb 6, 2023
 *      Author: tochk
 */

#include "HS420561.h"

#define		DIG1_GPIO				GPIOB
#define		DIG1_GPIO_PIN			LL_GPIO_PIN_5
#define		DIG2_GPIO				GPIOB
#define		DIG2_GPIO_PIN			LL_GPIO_PIN_6
#define		DIG3_GPIO				GPIOB
#define		DIG3_GPIO_PIN			LL_GPIO_PIN_7
#define		DIG4_GPIO				GPIOB
#define		DIG4_GPIO_PIN			LL_GPIO_PIN_8

#define		TIMER					TIM3
#define		ISR_PRIORITY			6

#define		TIMER_CLOCK()			LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3)
#define		TIMER_IRQHandler		TIM3_IRQHandler

#define		DIG_SHOW_PERIOD			100		// период отображения одного разряда (мкс)
#define		FLASH_PERIOD			200		// период отображения режима "мигающий" (мс)
#define		FLASH_CYCLE				(FLASH_PERIOD * 1000)/(DIG_SHOW_PERIOD)


#define DIG1_ON()		LL_GPIO_SetOutputPin(DIG1_GPIO, DIG1_GPIO_PIN)
#define DIG1_OFF()		LL_GPIO_ResetOutputPin(DIG1_GPIO, DIG1_GPIO_PIN)
#define DIG2_ON()		LL_GPIO_SetOutputPin(DIG2_GPIO, DIG2_GPIO_PIN)
#define DIG2_OFF()		LL_GPIO_ResetOutputPin(DIG2_GPIO, DIG2_GPIO_PIN)
#define DIG3_ON()		LL_GPIO_SetOutputPin(DIG3_GPIO, DIG3_GPIO_PIN)
#define DIG3_OFF()		LL_GPIO_ResetOutputPin(DIG3_GPIO, DIG3_GPIO_PIN)
#define DIG4_ON()		LL_GPIO_SetOutputPin(DIG4_GPIO, DIG4_GPIO_PIN)
#define DIG4_OFF()		LL_GPIO_ResetOutputPin(DIG4_GPIO, DIG4_GPIO_PIN)


/*	Маски символов под регистр HC595	*/
uint8_t	symbolMask[] = {
		0b00111111,			// '0'
		0b00000110,			// '1'
		0b01011011,			// '2'
		0b01001111,			// '3'
		0b01100110,			// '4'
		0b01101101,			// '5'
		0b01111101,			// '6'
		0b00000111,			// '7'
		0b01111111,			// '8'
		0b01101111,			// '9'
		0b10000000			// '.'
};

t_HS420561_Base* currentBase;

/* Текущее отображаемое значение на индикаторе */
typedef struct {
	uint8_t	D1;
	uint8_t	D2;
	uint8_t	D3;
	uint8_t	D4;
}	t_DispVal;
t_DispVal currentDispVal;

typedef enum {
	ON,
	OFF
} t_FlashState;
t_FlashState flashState;

__IO uint32_t	show_cycle_count = 0;	//счетчик числа циклов отображения


///////////////////////////////// Машина состояний драйвера индикатора ///////////////////////
/*	Вызов машины - каждое окончание периода отображения (isr update таймера)

/* Возможные состояния */
typedef enum {
	BEGIN,
	SHOW_DIG1,
	SHOW_DIG2,
	SHOW_DIG3,
	SHOW_DIG4,
	END
}	dispState_t;


__IO dispState_t 		dispState;			// текущие состояния энкодера

void fsBegin(void);
void fsShowDig1(void);
void fsShowDig2(void);
void fsShowDig3(void);
void fsShowDig4(void);
void fsEnd(void);


// Массив указателей на функции-обработчики состояний (реализация вызова машины состояний)
void (*fsDispState[])() = {	fsBegin,
							fsShowDig1,
							fsShowDig2,
							fsShowDig3,
							fsShowDig4,
							fsEnd};
/////////////////////////////////////////////////////////////////////////////////////


static void Timer_Init(void);
static void GPIO_Init(void);



/*
 * Прерывание таймера энкодера - обновление
 */
void TIMER_IRQHandler(void)
{

	if (LL_TIM_IsActiveFlag_UPDATE(TIMER))
	{
		LL_TIM_ClearFlag_UPDATE (TIMER);
		// Вызов МС
		show_cycle_count ++;
		fsDispState[dispState]();
	}
}





void HS420561_Init(t_HS420561_Base* base)
{
	currentBase = base;

	N74HC595_Init();

	Timer_Init();

	GPIO_Init();
	DIG1_OFF();
	DIG2_OFF();
	DIG3_OFF();
	DIG4_OFF();

	dispState = BEGIN;

	LL_TIM_EnableIT_UPDATE(TIMER);
	LL_TIM_EnableCounter(TIMER);
}


static void GPIO_Init(void)
{
	LL_GPIO_InitTypeDef	GPIO_InitStruct = {0};

	/* Digit 1 */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	GPIO_InitStruct.Pin = DIG1_GPIO_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(DIG1_GPIO, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(DIG1_GPIO, DIG1_GPIO_PIN);

	/* Digit 2 */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	GPIO_InitStruct.Pin = DIG2_GPIO_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(DIG2_GPIO, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(DIG2_GPIO, DIG2_GPIO_PIN);

	/* Digit 3 */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	GPIO_InitStruct.Pin = DIG3_GPIO_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(DIG3_GPIO, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(DIG3_GPIO, DIG3_GPIO_PIN);

	/* Digit 4 */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	GPIO_InitStruct.Pin = DIG4_GPIO_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(DIG4_GPIO, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(DIG4_GPIO, DIG4_GPIO_PIN);
}

/*
 * Инициализация таймера
 */
static void Timer_Init(void)
{
	LL_TIM_InitTypeDef 	TIM_InitStruct = 			{0};

	/*  Счетчик будет отсчитывать длительность включения одного разряда	индикатора	*/
	uint32_t period = (uint32_t)((SystemCoreClock/1000000) * DIG_SHOW_PERIOD);

	TIMER_CLOCK();
	TIM_InitStruct.Prescaler = 1 - 1;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = period - 1 ;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIMER, &TIM_InitStruct);
	// Прерывание по переполнению
	NVIC_SetPriority(TIM3_IRQn, ISR_PRIORITY);
	NVIC_EnableIRQ(TIM3_IRQn);
	if (LL_TIM_IsActiveFlag_UPDATE (TIMER))
	{
		LL_TIM_ClearFlag_UPDATE (TIMER);
	}

	//LL_TIM_EnableIT_UPDATE(ENCODER_TIMER);
	//LL_TIM_EnableCounter(ENCODER_TIMER);
}



void fsBegin(void)
{
	currentDispVal.D1 = symbolMask[currentBase->S1.val];
	if (currentBase->S1.point)
		currentDispVal.D1 |= symbolMask[10];

	currentDispVal.D2 = symbolMask[currentBase->S2.val];
	if (currentBase->S2.point)
		currentDispVal.D2 |= symbolMask[10];

	currentDispVal.D3 = symbolMask[currentBase->S3.val];
	if (currentBase->S3.point)
		currentDispVal.D3 |= symbolMask[10];

	currentDispVal.D4 = symbolMask[currentBase->S4.val];
	if (currentBase->S4.point)
		currentDispVal.D4 |= symbolMask[10];

	if ((show_cycle_count >= 0) && (show_cycle_count < FLASH_CYCLE))
		flashState = ON;
	else if ((show_cycle_count >= FLASH_CYCLE) && (show_cycle_count < FLASH_CYCLE*2))
		flashState = OFF;
	else if (show_cycle_count >= FLASH_CYCLE*2)
		show_cycle_count = 0;

	dispState = SHOW_DIG1;
}

void fsShowDig1(void)
{
	DIG4_OFF();
	//N74HC595_Write(symbolMask[currentBase->S1.val]);
	N74HC595_Write(currentDispVal.D1);
	if (currentBase->S1.mode == NORMAL)
	{
		DIG1_ON();
	}
	else if (currentBase->S1.mode == FLASHING)
	{
		if (flashState == ON)
			DIG1_ON();
	}
	dispState = SHOW_DIG2;
}

void fsShowDig2(void)
{
	DIG1_OFF();

	N74HC595_Write(currentDispVal.D2);
	if (currentBase->S2.mode == NORMAL)
	{
		DIG2_ON();
	}
	else if (currentBase->S2.mode == FLASHING)
	{
		if (flashState == ON)
			DIG2_ON();
	}
	dispState = SHOW_DIG3;
}

void fsShowDig3(void)
{
	DIG2_OFF();

	N74HC595_Write(currentDispVal.D3);
	if (currentBase->S3.mode == NORMAL)
	{
		DIG3_ON();
	}
	else if (currentBase->S3.mode == FLASHING)
	{
		if (flashState == ON)
			DIG3_ON();
	}
	dispState = SHOW_DIG4;
}

void fsShowDig4(void)
{
	DIG3_OFF();

	N74HC595_Write(currentDispVal.D4);
	if (currentBase->S4.mode == NORMAL)
	{
		DIG4_ON();
	}
	else if (currentBase->S4.mode == FLASHING)
	{
		if (flashState == ON)
			DIG4_ON();
	}
	dispState = END;
}

void fsEnd(void)
{
	DIG4_OFF();
	dispState = BEGIN;
}





