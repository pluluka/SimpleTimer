/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "HS420561.h"
#include "ReDefSTDIO.h"
#include "ExtButton.h"
#include "zummer.h"



///////////////////////////////// МC таймера ///////////////////////////////////
/*	Вызов МС - в основном цикле

/* Возможные состояния */
typedef enum {
	SHOW_COUNTER,		// показываем на дисплее значения счетчика энкодера
	SET_TIME_SEK,		// установка секунд
	SET_TIME_MIN,		// установка минут
	SET_COUNTER,		// установка значения счетчика энкодера как времени срабатывания
	SHOW_RTC_COUNTDOWN,	// показываем на дисплее обратный отсчет времени срабатывания
	ALARM_ON,			// включаем звуковой сигнал
	ALARM_TIME,			// сигнал
	RESET_COUNTER		// сброс
}	timerState_t;


__IO timerState_t 		timerState;		// текущие состояния 

void fsShowCounter(void);
void fsSetTimeSek(void);
void fsSetTimeMin(void);
void fsSetCounter(void);
void fsShowRTCCountdown(void);
void fsAlarmON(void);
void fsAlarmTime(void);
void fsResetCounter(void);


// Массив указателей на функции-обработчики состояний (реализация вызова машины состояний)
void (*fsTimerState[])() = {	fsShowCounter,
								fsSetTimeSek,
								fsSetTimeMin,
								fsSetCounter,
								fsShowRTCCountdown,
								fsAlarmON,
								fsAlarmTime,
								fsResetCounter};
/////////////////////////////////////////////////////////////////////////////////////

/* Текущие значения таймера и дисплея */
typedef struct{
	uint16_t	sek;
	uint16_t	min;
	uint16_t	wkp;		// установленное время (сек)
} t_Timer;
__IO t_Timer currentTimer = {0};

/* Кнопки */
t_ExtButton extBtn_StartReset = {0};
t_ExtButton extBtn_TimeSet = {0};

/* Дисплей */
t_HS420561_Base dispBase;



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void RTC_Init(uint32_t clock_LSE);
__STATIC_INLINE void RTC_START(void);
__STATIC_INLINE void RTC_STOP(void);
void ValSetEncoder_Init(void);
void ExtBtnStartReset_Init(void);
void ExtBtnTimeSet_Init(void);
//__STATIC_INLINE void Zummer_ON(void);
//__STATIC_INLINE void Zummer_OFF(void);
void displayShowTime(uint16_t timeInSekonds);

__STATIC_INLINE void Delay(__IO uint32_t nCount);




/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	/* System interrupt init*/
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* SysTick_IRQn interrupt configuration */
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

	/** DISABLE: JTAG-DP Disabled and SW-DP Disabled
	*/
	//0LL_GPIO_AF_DisableRemap_SWJ();

	/* Configure the system clock */
	SystemClock_Config();

	StdOutFile_Init();


	RTC_Init(32768);

  	// кнопка запуска/сброса
    t_BtnAntiBouncing antiBounc_SetReset;
	extBtn_StartReset.gpio = GPIOB;
	extBtn_StartReset.pin = LL_GPIO_PIN_1;
	extBtn_StartReset.type = EXT_BTN_TYPE_FALLING;
	extBtn_StartReset.BtnInitFunc = (void*)ExtBtnStartReset_Init;
	extBtn_StartReset.BthHandlerFunc = NULL;
	extBtn_StartReset.antiBouncing = &antiBounc_SetReset;
	ExtButton_Add(&extBtn_StartReset);
	// кнопка установки времени (выбор мин/сек)
    t_BtnAntiBouncing antiBounc_TimeSet;
	extBtn_TimeSet.gpio = GPIOB;
	extBtn_TimeSet.pin = LL_GPIO_PIN_9;
	extBtn_TimeSet.type = EXT_BTN_TYPE_FALLING;
	extBtn_TimeSet.BtnInitFunc = (void*)ExtBtnTimeSet_Init;
	extBtn_TimeSet.BthHandlerFunc = NULL;
	extBtn_TimeSet.antiBouncing = &antiBounc_TimeSet;
	ExtButton_Add(&extBtn_TimeSet);
	// запуск обработчика кнопок
	ExtButton_Init();			// TIM4

	// индикатор
	dispBase.S1.point = POINT_OFF;
	dispBase.S2.point = POINT_OFF;
	dispBase.S3.point = POINT_ON;
	dispBase.S4.point = POINT_OFF;
	HS420561_Init(&dispBase);	// TIM3

	t_ZummerBase zummerBase;
	zummerBase.freq = 2000;
	zummerBase.pulseTime = 500;
	ZummerInit(&zummerBase);	// TIM2
	ValSetEncoder_Init();		// TIM1

	timerState = RESET_COUNTER;

	extBtn_TimeSet.pressed = 0;

	/* Infinite loop */
	while (1)
	{
		fsTimerState[timerState]();
		Delay(2000);
	}

}

/*
 * Показывает время на дисплее в формате мин.сек
 */
void displayShowTime(uint16_t timeInSekonds)
{
	uint16_t timeMin = (uint16_t)(timeInSekonds / 60);
	uint16_t timeSek = (uint16_t)(timeInSekonds % 60);

	dispBase.S1.val = timeSek % 10;
	timeSek /= 10;
	dispBase.S2.val = timeSek % 10;

	dispBase.S3.val = timeMin % 10;
	timeMin /= 10;
	dispBase.S4.val = timeMin % 10;

	//dispBase.S3.val |= HS420561_DIG_POINT;
}

void fsShowCounter(void)
{
	displayShowTime(currentTimer.wkp);

	if (extBtn_TimeSet.pressed)
	{
		TIM1->CNT = 0;
		extBtn_TimeSet.pressed = 0;
		dispBase.S1.mode = FLASHING;
		dispBase.S2.mode = FLASHING;
		dispBase.S3.mode = NORMAL;
		dispBase.S4.mode = NORMAL;
		timerState = SET_TIME_SEK;
	}
}

void fsSetTimeSek(void)
{
	if ((TIM1->CNT / 2) > 59)
		TIM1->CNT = 0;
	currentTimer.sek = (TIM1->CNT / 2);
	displayShowTime(currentTimer.min*60 + currentTimer.sek);
	if (extBtn_StartReset.pressed)
	{
		extBtn_StartReset.pressed = 0;
		TIM1->CNT = 0;
		currentTimer.wkp = currentTimer.min*60 + currentTimer.sek;
		timerState = SET_COUNTER;
		return;
	}

	if (extBtn_TimeSet.pressed)
	{
		TIM1->CNT = currentTimer.min * 2;
		extBtn_TimeSet.pressed = 0;
		dispBase.S1.mode = NORMAL;
		dispBase.S2.mode = NORMAL;
		dispBase.S3.mode = FLASHING;
		dispBase.S4.mode = FLASHING;
		timerState = SET_TIME_MIN;
	}
}

void fsSetTimeMin(void)
{
	if ((TIM1->CNT / 2) > 99)
		TIM1->CNT = 0;
	currentTimer.min = (TIM1->CNT / 2);
	displayShowTime(currentTimer.min*60 + currentTimer.sek);
	if (extBtn_StartReset.pressed)
	{
		extBtn_StartReset.pressed = 0;
		TIM1->CNT = 0;
		currentTimer.wkp = currentTimer.min*60 + currentTimer.sek;
		timerState = SET_COUNTER;
		return;
	}

	if (extBtn_TimeSet.pressed)
	{
		TIM1->CNT = currentTimer.sek * 2;
		extBtn_TimeSet.pressed = 0;
		dispBase.S1.mode = FLASHING;
		dispBase.S2.mode = FLASHING;
		dispBase.S3.mode = NORMAL;
		dispBase.S4.mode = NORMAL;
		timerState = SET_TIME_SEK;
	}
}

void fsSetCounter(void)
{
	dispBase.S1.mode = NORMAL;
	dispBase.S2.mode = NORMAL;
	dispBase.S3.mode = NORMAL;
	dispBase.S4.mode = NORMAL;

	if ((currentTimer.wkp > 0 ) && (currentTimer.wkp < 5999))
	{
		RTC_START();
		timerState = SHOW_RTC_COUNTDOWN;
	}
	else
	{
		timerState = RESET_COUNTER;
	}
}

void fsShowRTCCountdown(void)
{
	uint32_t countRTC = (uint32_t)(RTC->CNTL);

	displayShowTime(currentTimer.wkp - countRTC);

	if (countRTC > (currentTimer.wkp - 1))
	{
		timerState = ALARM_ON;
		return;
	}
	if (extBtn_StartReset.pressed)
	{
		extBtn_StartReset.pressed = 0;
		timerState = RESET_COUNTER;
	}
}

void fsAlarmON(void)
{
	RTC_STOP();
	ZummerON();
	timerState = ALARM_TIME;
}

void fsAlarmTime(void)
{
	if (extBtn_StartReset.pressed)
	{
		extBtn_StartReset.pressed = 0;
		timerState = RESET_COUNTER;
	}
}

void fsResetCounter(void)
{
	ZummerOFF();

	currentTimer.wkp = 0;
	TIM1->CNT = 0;

	dispBase.S1.mode = NORMAL;
	dispBase.S2.mode = NORMAL;
	dispBase.S3.mode = NORMAL;
	dispBase.S4.mode = NORMAL;

	timerState = SHOW_COUNTER;
}

/*
 * Энкодер установки значения таймера
 */
void ValSetEncoder_Init(void)
{
	  LL_TIM_InitTypeDef TIM_InitStruct = {0};

	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* Peripheral clock enable */
	  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

	  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	  /**TIM1 GPIO Configuration
	  PA8   ------> TIM1_CH1
	  PA9   ------> TIM1_CH2
	  */
	  GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
	  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /* USER CODE BEGIN TIM1_Init 1 */

	  /* USER CODE END TIM1_Init 1 */
	  LL_TIM_SetEncoderMode(TIM1, LL_TIM_ENCODERMODE_X2_TI1);
	  LL_TIM_IC_SetActiveInput(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
	  LL_TIM_IC_SetPrescaler(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
	  LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
	  LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
	  LL_TIM_IC_SetActiveInput(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
	  LL_TIM_IC_SetPrescaler(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
	  LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
	  LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
	  TIM_InitStruct.Prescaler = 0;
	  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP | LL_TIM_COUNTERMODE_DOWN;
	  TIM_InitStruct.Autoreload = 9999;
	  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	  TIM_InitStruct.RepetitionCounter = 0;
	  LL_TIM_Init(TIM1, &TIM_InitStruct);
	  LL_TIM_DisableARRPreload(TIM1);
	  LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
	  LL_TIM_DisableMasterSlaveMode(TIM1);
	  LL_TIM_EnableCounter(TIM1);


}


void SystemClock_Config(void)
{
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
	while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
	{
	}
	LL_RCC_HSE_Enable();

	/* Wait till HSE is ready */
	while(LL_RCC_HSE_IsReady() != 1)
	{

	}
	LL_PWR_EnableBkUpAccess();
	if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
	{
		LL_RCC_ForceBackupDomainReset();
		LL_RCC_ReleaseBackupDomainReset();
	}
	LL_RCC_LSE_Enable();

	/* Wait till LSE is ready */
	while(LL_RCC_LSE_IsReady() != 1)
	{

	}
	if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
	{
		LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
	}
	LL_RCC_EnableRTC();
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
	LL_RCC_PLL_Enable();

	/* Wait till PLL is ready */
	while(LL_RCC_PLL_IsReady() != 1)
	{

	}
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	/* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
	{

	}
	LL_Init1msTick(72000000);
	LL_SetSystemCoreClock(72000000);
}

/**
  RTC клок от внешнего 32768 кварца, 1 сек. - тик счетчика
  */
void RTC_Init(uint32_t clock_LSE)
{

	LL_RTC_InitTypeDef RTC_InitStruct = {0};

	LL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_BKP);
	/* Peripheral clock enable */
	LL_RCC_EnableRTC();

	/* RTC interrupt Init */
	//NVIC_SetPriority(RTC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	//NVIC_EnableIRQ(RTC_IRQn);

	/** Initialize RTC and set the Time and Date
	*/
	RTC_InitStruct.AsynchPrescaler = clock_LSE - 1;
	LL_RTC_Init(RTC, &RTC_InitStruct);
	LL_RTC_SetAsynchPrescaler(RTC, clock_LSE - 1);

	RTC_STOP();
}

__STATIC_INLINE void RTC_START(void)
{
	LL_RCC_EnableRTC();
	LL_RTC_TIME_SetCounter(RTC, 0);
}

__STATIC_INLINE void RTC_STOP(void)
{
	LL_RTC_TIME_SetCounter(RTC, 0);
	LL_RCC_DisableRTC();
}


/*
 * Кнопка старта/сброса таймера
 * Функция инициализации
 */
void ExtBtnStartReset_Init(void)
{
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/*
 * Кнопка установки времени таймера
 * Функция инициализации
 */
void ExtBtnTimeSet_Init(void)
{
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


__STATIC_INLINE void Delay(__IO uint32_t nCount)
{
	nCount *= 20;
    while(nCount--)
    {
	    __asm("NOP");
    }
}
