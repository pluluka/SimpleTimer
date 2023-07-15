/*
 * ReDefSTDIO.h
 *
 *  Created on: 13 янв. 2021 г.
 *      Author: tochk
 *      Переопределяем стандартный поток вывода printf
 *      (например в USART)
 */

#ifndef SRC_REDEFSTDIO_H_
#define SRC_REDEFSTDIO_H_



#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILENO
#include "stm32f1xx_ll_usart.h"


// сюда будем направлять поток
#define STDOUT_USART		USART3


void USART_Init(USART_TypeDef *USARTx, uint32_t BaudRate)
{
	LL_USART_InitTypeDef USART_InitStruct = {0};


	/* Peripheral clock enable */
	if (USARTx == USART1)
	{
		  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
	}
	else if (USARTx == USART2)
	{
		  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
	}
	else if (USARTx == USART3)
	{
		  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
	}


		LL_GPIO_InitTypeDef GPIO_InitStruct = {0};


		if (USARTx == USART1)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
			  /**USART1 GPIO Configuration
			  PA9   ------> USART1_TX
			  PA10   ------> USART1_RX
			  */
			  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
			  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
			  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
			  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if (USARTx == USART2)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
			  /**USART2 GPIO Configuration
			  PA2   ------> USART2_TX
			  PA3   ------> USART2_RX
			  */
			  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
			  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
			  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
			  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
		else if (USARTx == USART3)
		{
			LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
			  /**USART3 GPIO Configuration
			  PB10   ------> USART3_TX
			  PB11   ------> USART3_RX
			  */
			  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
			  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
			  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
			  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
			  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
			  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		}


	/* USART initialization and enable */
	USART_InitStruct.BaudRate = 9600;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	//USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_8;
	LL_USART_Init(USARTx, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(USARTx);
	LL_USART_Enable(USARTx);

}

__STATIC_INLINE USART_SendByte(USART_TypeDef* USARTx, uint8_t data)
{
	// ждем пока сдвиговый регистр выплюнет данные
    while(!(USARTx->SR & USART_SR_TC))
    {
    	;
    }
    USARTx->DR = data;
}




int _write(int file, char *data, int len)
{
   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
   {
      errno = EBADF;
      return -1;
   }
   for (int i = 0; i < len; i ++)
   {
	   if (*data == '\n')
	   {
		   USART_SendByte(STDOUT_USART, '\n');
		   USART_SendByte(STDOUT_USART, '\r');
	   }
	   else
	   {
		   USART_SendByte(STDOUT_USART, *data++);
	   }
   }

   return 0;
}




void StdOutFile_Init(void)
{
	USART_Init(STDOUT_USART, 9600);
	printf("\n\n##### DEBUG ON %08d INIT DONE #####\n", STDOUT_USART);
}



#endif /* SRC_REDEFSTDIO_H_ */
