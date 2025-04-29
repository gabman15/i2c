#include "led.h"

void LED_Init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    //Clear Output Modes of pins 12-15
    GPIOD->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER14 | GPIO_MODER_MODER15);
    //Set Output Mode of pins 12-15 to output
    GPIOD->MODER |= GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0;
    GPIOD->ODR &= ~((1UL<<GREEN_LED) | (1UL<<ORANGE_LED) | (1UL<<RED_LED) | (1UL<<BLUE_LED)); // Off all Leds
}
/* Private Function*/
static void LED_On(uint8_t pin) {
    GPIOD->ODR |= (1UL << pin);
}

static void LED_Off(uint8_t pin) {
    GPIOD->ODR &= ~(1UL << pin);
}

static void LED_Toggle(uint8_t pin) {
    GPIOD->ODR ^= (1UL << pin);
}

/* Public Functions*/
void LED_Blue_On() {
    LED_On(BLUE_LED);
}
void LED_Blue_Off() {
    LED_Off(BLUE_LED);
}
void LED_Blue_Toggle() {
    LED_Toggle(BLUE_LED);
}

void LED_Green_On() {
    LED_On(GREEN_LED);
}
void LED_Green_Off() {
    LED_Off(GREEN_LED);
}
void LED_Green_Toggle() {
    LED_Toggle(GREEN_LED);
}

void LED_Red_On() {
    LED_On(RED_LED);
}
void LED_Red_Off() {
    LED_Off(RED_LED);
}
void LED_Red_Toggle() {
    LED_Toggle(RED_LED);
}

void LED_Orange_On() {
    LED_On(ORANGE_LED);
}
void LED_Orange_Off() {
    LED_Off(ORANGE_LED);
}
void LED_Orange_Toggle() {
    LED_Toggle(ORANGE_LED);
}
