/*
 * benchmark.h
 *
 *  Created on: Nov 25, 2022
 *      Author: fef0
 */

#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"

#ifndef INC_BENCHMARK_H_
#define INC_BENCHMARK_H_

void TIM_init(TIM_TypeDef * tim);
void TIM_config_timebase(TIM_TypeDef * tim, uint32_t prescaler, uint32_t autoreload);
void TIM_on(TIM_TypeDef* tim);


#endif /* INC_BENCHMARK_H_ */
