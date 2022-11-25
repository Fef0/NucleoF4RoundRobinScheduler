/*
 * benchmark.c
 *
 *  Created on: Nov 25, 2022
 *      Author: fef0
 */

#include "benchmark.h"
#include "scheduler.h"

void TIM_init(TIM_TypeDef * tim) {
    // enable timer clock
	if (tim == TIM1)
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	else if (tim == TIM2)
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	else if (tim == TIM3)
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	else if (tim == TIM4)
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	else if (tim == TIM5)
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

	tim->CR1 = 0; // timer off & update events off
	tim->SMCR = 0;
	tim->EGR = 0;
	tim->RCR = 0;
	tim->DIER = 0;
	tim->SR = 0;
}

void TIM_config_timebase(TIM_TypeDef * tim, uint32_t prescaler, uint32_t autoreload) {
	tim->PSC = prescaler - 1;
	tim->ARR = autoreload;
	tim->CNT = 0;
}

void TIM_on(TIM_TypeDef* tim) {
	tim->CR1 |= TIM_CR1_CEN;
	tim->SR = 0;
}

uint32_t runtime = 0;

void task0() {
	runtime = TIM2->CNT;
	while(1);
}

void task1() {
	runtime = TIM2->CNT;
	while(1);
}

int main2(void)
{
  /*
   * System init functions, omitted for space saving
   */
  addTask(&task0);

  addTask(&task1);

  // Start scheduler with 1ms time quanta
  startScheduler(1);

  return 0;
}
