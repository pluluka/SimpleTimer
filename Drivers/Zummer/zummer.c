/*
 * zummer.c
 *
 *  Created on: 8 мар. 2023 г.
 *      Author: tochk
 */


#include "zummer.h"


__IO uint32_t	__zummmer_freq_counter;	// счетчик периодов сигнала зуммера
t_ZummerBase* zummer;



/*
 * Прерывание таймера - обновление
 */
void TIM2_IRQHandler(void)
{

	if (LL_TIM_IsActiveFlag_UPDATE(TIM2))
	{
		LL_TIM_ClearFlag_UPDATE (TIM2);
		__zummmer_freq_counter ++;
		if (__zummmer_freq_counter > ((zummer->freq *  zummer->pulseTime) / 1000))
		{
			__zummmer_freq_counter = 0;
			if (LL_TIM_CC_IsEnabledChannel(TIM2, LL_TIM_CHANNEL_CH1))
				LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
			else
				LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
		}
	}
}


void ZummerInit(t_ZummerBase* zummBase)
{
	zummer = zummBase;
	uint32_t pwmPeriod = (uint32_t)(SystemCoreClock / zummer->freq);

	if (pwmPeriod > 0xFFFF)
		return;

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = pwmPeriod - 1;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = (uint32_t)(pwmPeriod / 2) - 1;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  // Прерывание по переполнению откл
  LL_TIM_EnableIT_UPDATE(TIM2);
  NVIC_EnableIRQ(TIM2_IRQn);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**TIM2 GPIO Configuration
  PA0-WKUP   ------> TIM2_CH1
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);

  //LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
  //LL_TIM_EnableCounter(TIM2);
  __zummmer_freq_counter = 0;
}

void ZummerON(void)
{
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
	LL_TIM_EnableCounter(TIM2);
}

void ZummerOFF(void)
{
	LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH1);
	LL_TIM_DisableCounter(TIM2);
}
