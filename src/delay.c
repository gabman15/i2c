#include "delay.h"

uint32_t maxticks = 0;

void DELAY_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    TIM4->PSC = 0;
    TIM4->ARR = 16000;
    TIM4->DIER |= TIM_DIER_UIE;
    TIM4->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM4_IRQn);
    maxticks = 0;
}

void TIM4_IRQHandler(void)
{
    maxticks++;
    TIM4->SR = 0;
}

void DELAY_ms(uint16_t delay){
    maxticks = 0;
    while (maxticks < delay) {}
}
