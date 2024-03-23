#include "main.h"

void System_Clock_Init() {
    
}

void i2c_init() {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Enable i2c clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable GPIOB clock (where i2c pins are)

    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9); // Clear mode of PB8 and PB9
    GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
    //Set mode of PB8 and PB9 to alternate functions

    GPIOB->OTYPER |= GPIO_OTYPE_OT8 | GPIO_OTYPE_OT9;
    // Set output type of PB8 and PB9 to open drain

    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED8_0 | GPIO_OSPEEDR_OSPEED8_1) | \
                      (GPIO_OSPEEDR_OSPEED9_0 | GPIO_OSPEEDR_OSPEED9_1);
    // Set speed of PB8 and PB9 to High Speed
    
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9); // Clear pull up mode for PB8 and PB9
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0;
    // Set PB8 and PB9 to have internal pull up resistors

    GPIOB->AFRH &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9); // Clear alternate function on PB8 and PB9
    GPIOB->AFRH |= (4U << GPIO_AFRH_AFSEL8_Pos) | (4U << GPIO_AFRH_AFSEL9_Pos);
    // SET PB8 and PB9 to AF4 (corresponding to I2C SCL and SDA respectively)

    
}
