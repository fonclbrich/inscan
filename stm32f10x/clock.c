/*
 * clock.c
 *
 *  Created on: 24 jan. 2018
 *      Author: erik.welander
 */

//#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_flash.h>

void clockInit()
{
	  /* The HSE pins are located on port D on pins PD0 and PD1. This means we
	     * need to enable the port clock to port D */
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;

	    /* Turn on HSE */
	RCC->CR |= RCC_CR_HSEON;

	    /* Wait for the HSE to stabilize */
	unsigned int n;

	for (n = 0; n != 0xFFFF && (RCC->CR & RCC_CR_HSERDY) == 0; n++) __asm("nop");

	RCC->CFGR |= RCC_CFGR_PLLMULL9;

		/* Set the PLL source to the HSE (8 MHz external crystal) */
	RCC->CFGR |= RCC_CFGR_PLLSRC;

	    /* Turn on PLL */
	RCC->CR |= RCC_CR_PLLON;

	    /* Wait the PLL it to stabilize */
	for (n = 0; n != 0xFFFF && (RCC->CR & RCC_CR_PLLRDY) == 0; n++) __asm("nop");

	/* Set the APB1 bus prescaler to /2 as 36 MHz it its max documented speed */
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

	/* Set the flash latency to two wait-states as required for 72 MHz SYSCLK */
	FLASH->ACR |= FLASH_ACR_LATENCY_1;

	/* Set the system clock to PLL */
	RCC->CFGR |= RCC_CFGR_SW_PLL;
}

