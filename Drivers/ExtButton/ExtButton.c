/*
 * ExtButton.c
 *
 *  Created on: Mar 1, 2023
 *      Author: tochk
 */

#include "ExtButton.h"


#define		TIMER						TIM4
#define		TIMER_ISR_PRIORITY			6

#define		ANTIBOUNCING_CYCLE_TIME		10		// период опроса алгоритма антидребезга (мс)
#define		ANTIBOUNCING_CYCLE_CNT		4		// число опросов алгоритма антидребезга

#define		EXTBTNS_MAX_COUNT			10




static void Timer_Init(uint32_t interval);
static void AntiBouncing(__IO t_ExtButton* btn);



t_ExtButton* 	extBtns[EXTBTNS_MAX_COUNT] = {NULL};
uint32_t		extBtnsCurrentCount = 0;


/*
 * Прерывание таймера обработки нажатий кнопок
 */
void TIM4_IRQHandler(void)
{
	if (LL_TIM_IsActiveFlag_UPDATE(TIM4))
	{
		LL_TIM_ClearFlag_UPDATE(TIM4);

		t_ExtButton** currExtBtn = extBtns;
		while (*currExtBtn != NULL)
		{
			AntiBouncing(*currExtBtn);
			currExtBtn ++;
		}

	}
}


void ExtButton_Init(void)
{
	Timer_Init(ANTIBOUNCING_CYCLE_TIME);
}

void ExtButton_Add(t_ExtButton* extBtn)
{
	extBtn->BtnInitFunc();
	extBtn->state = LL_GPIO_IsOutputPinSet(extBtn->gpio, extBtn->pin);
	extBtn->antiBouncing->cnt = 0;
	extBtn->antiBouncing->prev_state = extBtn->state;

	extBtns[extBtnsCurrentCount] = extBtn;

	extBtnsCurrentCount ++;
}

/**
 * Таймер функции антидребезга кнопок
 */
static void Timer_Init(uint32_t interval)
{
	if ((interval == 0) || (interval > 200))
			return;

	LL_TIM_InitTypeDef TIM_InitStruct = {0};

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	TIM_InitStruct.Prescaler = SystemCoreClock / 100000;
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 100 * interval;
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM4, &TIM_InitStruct);


	// Прерывание по переполнению
	LL_TIM_EnableIT_UPDATE(TIM4);
	NVIC_EnableIRQ(TIM4_IRQn);

	LL_TIM_EnableCounter(TIM4);
}

/*
 * Алгоритм антидребезга
 */
static void AntiBouncing(__IO t_ExtButton* btn)
{
	uint8_t curr_state = LL_GPIO_IsInputPinSet(btn->gpio, btn->pin);
	// зафиксировано изменение состояния
	if (curr_state != btn->antiBouncing->prev_state)
	{
		if (btn->antiBouncing->cnt == 0)
		{
			btn->antiBouncing->cnt ++;
		}
		else
		{
			btn->antiBouncing->cnt = 0;
		}
		btn->antiBouncing->prev_state = curr_state;
	}
	else
	{
		if ((btn->antiBouncing->cnt > 0) && (btn->antiBouncing->cnt < 4))
		{
			btn->antiBouncing->cnt ++;
		}
		else if (btn->antiBouncing->cnt == 4)
		{
			btn->state = (btn->state == 0) ? 1 : 0;
			// зафиксирован устойчивый переход с высокого уровня на низкий
			if (btn->state == 0)
			{
				if (btn->type & EXT_BTN_TYPE_FALLING)
				{
					btn->pressed = 1;
					if (btn->BthHandlerFunc != NULL)
					{
						btn->BthHandlerFunc();
					}
				}
			}
			// зафиксирован устойчивый переход с низкого уровня на высокий
			else if (btn->state == 1)
			{
				if (btn->type & EXT_BTN_TYPE_RISING)
				{
					btn->pressed = 1;
					if (btn->BthHandlerFunc != NULL)
					{
						btn->BthHandlerFunc();
					}
				}
			}

			btn->antiBouncing->cnt = 0;
		}
	}
}


