/*
 * timer.c
 *
 *  Created on: 24 jan. 2018
 *      Author: erik.welander
 */

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>

volatile uint32_t timerCounter = 0;

void TIM2_IRQHandler()
{
	TIM2->SR &= ~TIM_SR_UIF;
	timerCounter++;
}

void waitSyncTimer()
{
	while (timerCounter == 0) {}
	timerCounter = 0;
}

void timerInit()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Seems to use 8 MHz clock, if nothing has been tampered with */
    TIM2->PSC = 40000-1;
    TIM2->ARR = 400;

    TIM2->DIER = TIM_DIER_UIE;
    TIM2->CR1 |= TIM_CR1_CEN;

	NVIC_EnableIRQ(TIM2_IRQn);
}
