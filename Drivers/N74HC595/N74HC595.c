/*
 * N74HC595.c
 *
 *  Created on: 4 февр. 2023 г.
 *      Author: tochk
 */

#include "N74HC595.h"



#define STCP_SET()        LL_GPIO_SetOutputPin(N74HC595_GPIO_STCP, N74HC595_GPIO_STCP_PIN)
#define STCP_RESET()      LL_GPIO_ResetOutputPin(N74HC595_GPIO_STCP, N74HC595_GPIO_STCP_PIN)


static void SPI_Init(SPI_TypeDef* SPIx);
static void GPIO_Init(void);
__STATIC_INLINE void spi_WriteData(SPI_TypeDef *SPIx, uint8_t data);
__STATIC_INLINE void spi_WaitForTransmisionEnd(SPI_TypeDef* SPIx);

__STATIC_INLINE void Delay(__IO uint32_t nCount);




void N74HC595_Init(void)
{
	SPI_Init(N74HC595_SPI);
	GPIO_Init();
}

void N74HC595_Write(uint8_t data)
{
	//STCP_RESET();
	spi_WriteData(N74HC595_SPI, data);
	spi_WaitForTransmisionEnd(N74HC595_SPI);
	STCP_SET();
	STCP_RESET();
}



static void SPI_Init(SPI_TypeDef *SPIx)
{
	  LL_SPI_InitTypeDef SPI_InitStruct = {0};
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};


	  if (SPIx == SPI1)
	  {

		  /* Peripheral clock enable */
		  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
		  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
		  /**SPI1 GPIO Configuration
		  PA5   ------> SPI1_SCK
		  PA6   ------> SPI1_MISO
		  PA7   ------> SPI1_MOSI
		  */
		  GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_7;
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
		  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
		  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  }
	  else if (SPIx == SPI2)
	  {

		  /* Peripheral clock enable */
		  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
		  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

		  /**SPI2 GPIO Configuration
		  PB13   ------> SPI2_SCK
		  PB14   ------> SPI2_MISO
		  PB15   ------> SPI2_MOSI
		  */
		  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_15;
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
		  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		  GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
		  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
		  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  }

	  /* SPI2 parameter configuration*/
	  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
	  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
	  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
	  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	  SPI_InitStruct.CRCPoly = 7; //????  (7)

	  if (LL_SPI_IsEnabled(SPIx) == 0x00000001U)
		  LL_SPI_Disable(SPIx);
	  LL_SPI_Init(SPIx, &SPI_InitStruct);

	  LL_SPI_Enable(SPIx);
}


__STATIC_INLINE void spi_WriteData(SPI_TypeDef *SPIx, uint8_t data)
{
	//ждём пока регистр DR скинет данные в сдвиговый регистр
	while(!(SPIx->SR & SPI_SR_TXE)) ;

	//отправляем данные
	LL_SPI_TransmitData8(SPIx, data);
}

__STATIC_INLINE void spi_WaitForTransmisionEnd(SPI_TypeDef* SPIx)
// ожидание окончания передачи
{
	while(SPIx->SR & SPI_SR_BSY);
}


static void GPIO_Init(void)
{

	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	if (N74HC595_GPIO_STCP == GPIOA)
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	else if (N74HC595_GPIO_STCP == GPIOB)
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	GPIO_InitStruct.Pin = N74HC595_GPIO_STCP_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(N74HC595_GPIO_STCP, &GPIO_InitStruct);
	STCP_RESET();
}


__STATIC_INLINE void Delay(__IO uint32_t nCount)
{
	nCount *= 20;
    while(nCount--)
    {
	    __asm("NOP");
    }
}



