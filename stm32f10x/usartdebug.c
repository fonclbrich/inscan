/*
 * stm32f10x_usart.cc
 *
 *  Created on: 17 jan. 2018
 *      Author: erik.welander
 */

#include "debug.h"
#include <stm32f10x.h>
// #include <stm32f10x_usart.h>

int debugInit()
{
	// Prepare USART2

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	GPIOA->CRL |= GPIO_CRL_MODE2;   //Output mode, max 50 MHz
	GPIOA->CRL &= ~GPIO_CRL_CNF2;   //Clear bits
	GPIOA->CRL |= GPIO_CRL_CNF2_1;  //Alternate function push-pull

	GPIOA->CRL &= ~GPIO_CRL_MODE3; //Input mode
	GPIOA->CRL &= ~GPIO_CRL_CNF3;  //Clear bits
	GPIOA->CRH |= GPIO_CRL_CNF3_0; //Floating input

    /* Set the baudrate for USART2
    * USART_BRR = bus_frequency / target_baud_rate
    * this example: 36000000/57600 = 625 */
    USART2->BRR =  625;

    /* Enable USART1 and its RX- and TX-component */
    USART2->CR1 |= USART_CR1_UE // Usart enable
        | USART_CR1_TE          // Transmitter enable
        | USART_CR1_RE         // Receiver enable
    	| USART_CR1_TXEIE;		// Interrupt enable

	debugSendString("Debugging via USART2.\n");


#ifdef PRINTCLKINFO
#include <stm32f10x_rcc.h>

	char numBuf[0x10];
	numBuf[0x0F] = 0;

	RCC_ClocksTypeDef RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);

	debugSendString("SYSCLK_Frequency: ");
	debugSendString(int2str(RCC_ClocksStatus.SYSCLK_Frequency, &numBuf[0x0E]));

	debugSendString("\nHCLK_Frequency: ");
	debugSendString(int2str(RCC_ClocksStatus.HCLK_Frequency, &numBuf[0x0E]));

	debugSendString("\nPCLK1_Frequency: ");
	debugSendString(int2str(RCC_ClocksStatus.PCLK1_Frequency, &numBuf[0x0E]));

	debugSendString("\nPCLK2_Frequency: ");
	debugSendString(int2str(RCC_ClocksStatus.PCLK2_Frequency, &numBuf[0x0E]));

	debugSendString("\nADCCLK_Frequency: ");
	debugSendString(int2str(RCC_ClocksStatus.ADCCLK_Frequency, &numBuf[0x0E]));

	debugSendString("\nUSART2->BRR: ");
	debugSendString(int2str( USART2->BRR & 0xFFFF, &numBuf[0x0E]));

#endif



	return 0;
}

void debugSend(char c)
{
	if (USART2->SR & USART_SR_TXE)
		USART2->DR = c;
}

#ifndef USARTDEBUGBUFFERSIZE
#define USARTDEBUGBUFFERSIZE 0x1000
#endif

static char usart_db_buf[USARTDEBUGBUFFERSIZE];
static int writeIndex = USARTDEBUGBUFFERSIZE - 1;
static int readIndex = USARTDEBUGBUFFERSIZE - 1;

void debugSendString(char *string)
{
	int n = 0;
	while (string[n] != 0)
	{
		usart_db_buf[writeIndex] = string[n++];
		writeIndex = writeIndex != 0 ? writeIndex - 1 : USARTDEBUGBUFFERSIZE - 1;
	}

	NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_IRQHandler()
{
	if (writeIndex != readIndex)
	{
		uint32_t itmask = USART2->CR1 & USART_CR1_TXEIE; // without this intermediate step of itmask, it does not work, why?
		if ((itmask != (uint16_t)RESET) && ((USART2->SR & USART_SR_TXE) != (uint16_t)RESET))
		{
			USART2->DR = usart_db_buf[readIndex];
			readIndex = readIndex != 0 ? readIndex - 1 : USARTDEBUGBUFFERSIZE - 1;
		}
	}
	else  NVIC_DisableIRQ(USART2_IRQn);
}

char *int2str(uint32_t N, char *bufEnd)
{
	if (N == 0)
	{
		*bufEnd = '0';
		return bufEnd;
	}

	while (N)
	{
		*(bufEnd--) = '0' + (N % 10);
		N /= 10;
	}

	return bufEnd + 1;
}

char *hex2str(uint32_t N, char *bufEnd)
{
	static char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	if (N == 0)
	{
		*bufEnd = '0';
		return bufEnd;
	}

	while (N)
	{
		*(bufEnd--) = digits[N & 0xF];
		N >>= 4;
	}

	return bufEnd + 1;
}

char *Dhex2str(uint32_t N)
{
	static char numBuf[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	return hex2str(N, numBuf + 7);
}
