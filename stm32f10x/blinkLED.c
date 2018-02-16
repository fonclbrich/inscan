/*
 * blinkLED.c
 *
 *  Created on: 16 jan. 2018
 *      Author: erik.welander
 */

#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

void blinkLEDinit()
{
	  /* Enable clock to IO port C */
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	    /* Set pin C13 as an output */
	GPIOC->CRH &= ~GPIO_CRH_MODE13; //Clear bits
	GPIOC->CRH |= GPIO_CRH_MODE13;  //Output mode, max 50 MHz
	GPIOC->CRH &= ~GPIO_CRH_CNF13;  //GPIO output push-pull
}

void blinkLEDset()
{
	GPIOC->BSRR = (1U << 13U);
}

void blinkLEDreset()
{
	GPIOC->BRR = (1U << 13U);
}
