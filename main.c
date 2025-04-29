#include "main.h"

void System_Clock_Init() {
    RCC->CR |= RCC_CR_HSEON; // Enable HSE

    while (!(RCC->CR & RCC_CR_HSERDY)); // Wait till HSE is ready

    RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Enable power interface clock

    PWR->CR |= PWR_CR_VOS; // Set voltage scale to 1 to get the max 168 Mhz

    RCC->CFGR &= ~RCC_CFGR_HPRE; // Clear AHB Prescaler setting
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // Don't divide sysclk

    RCC->CFGR &= ~RCC_CFGR_PPRE1; // Clear APB1 Prescaler setting
    RCC->CFGR &= ~RCC_CFGR_PPRE2; // Clear APB2 Prescaler setting

    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; // Divide sysclk by 4 for APB1
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; // Divide sysclk by 2 for APB2

    RCC->PLLCFGR |= (PLL_M << RCC_PLLCFGR_PLLM_Pos) | (PLL_N << RCC_PLLCFGR_PLLN_Pos) | \
        (PLL_P << RCC_PLLCFGR_PLLP_Pos) | RCC_PLLCFGR_PLLSRC | (PLL_Q << RCC_PLLCFGR_PLLQ_Pos);
    // Set PLL dividers and PLL SRC to HSE

    RCC->CR |= RCC_CR_PLLON; // Turn main PLL clock on

    while (!(RCC->CR & RCC_CR_PLLRDY)); // Wait till main PLL clock on

    FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | (FLASH_ACR_LATENCY_5WS << FLASH_ACR_LATENCY_Pos);
    // Configure flash latency settings

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL; // Set system clock to use main PLL

    while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)); // Wait till system clock is using PLL
}

void i2c_init() {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Enable i2c clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable GPIOB clock (where i2c pins are)

    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9); // Clear mode of PB8 and PB9
    GPIOB->MODER |= GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1;
    //Set mode of PB8 and PB9 to alternate functions

    GPIOB->OTYPER |= GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9;
    // Set output type of PB8 and PB9 to open drain

    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED8_0 | GPIO_OSPEEDR_OSPEED8_1) | \
        (GPIO_OSPEEDR_OSPEED9_0 | GPIO_OSPEEDR_OSPEED9_1);
    // Set speed of PB8 and PB9 to High Speed
    
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9); // Clear pull up mode for PB8 and PB9
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0;
    // Set PB8 and PB9 to have internal pull up resistors

    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL8 | GPIO_AFRH_AFSEL9); // Clear alternate function on PB8 and PB9
    GPIOB->AFR[1] |= (4U << GPIO_AFRH_AFSEL8_Pos) | (4U << GPIO_AFRH_AFSEL9_Pos);
    // SET PB8 and PB9 to AF4 (corresponding to I2C SCL and SDA respectively)

    //FINISHED CONFIGURATION OF PINS

    I2C1->CR1 = I2C_CR1_SWRST; // Reset i2c
    I2C1->CR1 = 0; // Clear i2c control register

    I2C1->CR2 |= I2C_CR2_ITERREN; // Enable i2c error interrupt
    
    I2C1->CR2 |= (10U << I2C_CR2_FREQ_Pos); // Set clock freq to 10mhz

    //Want 100kHz
    // For SM Mode:
    //    Thigh = CCR * TPCLK1
    //    Tlow  = CCR * TPCLK1
    // So to generate 100 kHz SCL frequency
    // we need 1/100kz = 10us clock speed
    // Thigh and Tlow needs to be 5us each
    // Let's pick fPCLK1 = 10Mhz, TPCLK1 = 1/10Mhz = 100ns
    // Thigh = CCR * TPCLK1 => 5us = CCR * 100ns
    // CCR = 50

    I2C1->CCR |= (50U << I2C_CCR_CCR_Pos);

    I2C1->TRISE |= (11U << I2C_TRISE_TRISE_Pos);

    I2C1->OAR1 |= (0x00 << I2C_OAR1_ADD1_Pos); // Set address mode to 7-bit
    I2C1->OAR1 |= (1 << 14);
    // enable error interrupt from NVIC
    NVIC_SetPriority(I2C1_ER_IRQn, 1);
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    I2C1->CR1 |= I2C_CR1_PE; // enable i2c
}

void i2c_init_2() {
    //*******************************
    // setup I2C - GPIOB 6, 9
    //*******************************
    // enable I2C clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // setup I2C pins
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~(3U << 6*2); // PB6
    GPIOB->MODER |=  (2 << 6*2); // AF
    GPIOB->OTYPER |= (1 << 6);   // open-drain
    GPIOB->MODER &= ~(3U << 9*2); // PB9
    GPIOB->MODER |=  (2 << 9*2); // AF
    GPIOB->OTYPER |= (1 << 9);   // open-drain

    // choose AF4 for I2C1 in Alternate Function registers
    GPIOB->AFR[0] |= (4 << 6*4);     // for pin 6
    GPIOB->AFR[1] |= (4 << (9-8)*4); // for pin 9

    // reset and clear reg
    I2C1->CR1 = I2C_CR1_SWRST;
    I2C1->CR1 = 0;

    I2C1->CR2 |= (I2C_CR2_ITERREN); // enable error interrupt

    // fPCLK1 must be at least 2 Mhz for SM mode
    //        must be at least 4 Mhz for FM mode
    //        must be multiple of 10Mhz to reach 400 kHz
    // DAC works at 100 khz (SM mode)
    // For SM Mode:
    //    Thigh = CCR * TPCLK1
    //    Tlow  = CCR * TPCLK1
    // So to generate 100 kHz SCL frequency
    // we need 1/100kz = 10us clock speed
    // Thigh and Tlow needs to be 5us each
    // Let's pick fPCLK1 = 10Mhz, TPCLK1 = 1/10Mhz = 100ns
    // Thigh = CCR * TPCLK1 => 5us = CCR * 100ns
    // CCR = 50
    I2C1->CR2 |= (10 << 0); // 10Mhz periph clock
    I2C1->CCR |= (50 << 0);
    // Maximum rise time.
    // Calculation is (maximum_rise_time / fPCLK1) + 1
    // In SM mode maximum allowed SCL rise time is 1000ns
    // For TPCLK1 = 100ns => (1000ns / 100ns) + 1= 10 + 1 = 11
    I2C1->TRISE |= (11 << 0); // program TRISE to 11 for 100khz
    // set own address to 00 - not really used in master mode
    I2C1->OAR1 |= (0x00 << 1);
    I2C1->OAR1 |= (1 << 14); // bit 14 should be kept at 1 according to the datasheet

    // enable error interrupt from NVIC
    NVIC_SetPriority(I2C1_ER_IRQn, 1);
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    I2C1->CR1 |= I2C_CR1_PE; // enable i2c
}

static inline void __i2c_start() {
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
}

static inline void __i2c_stop() {
    I2C1->CR1 |= I2C_CR1_STOP;
    while (!(I2C1->SR2 & I2C_SR2_BUSY));
}

void i2c_write_byte(uint8_t blocksel, uint8_t highaddr, uint8_t lowaddr, uint8_t data) {
    __i2c_start();

    uint8_t control_byte = EEPROM_ADDR;

    if (blocksel)
        control_byte |= EEPROM_BLOCK_SEL;

    I2C1->DR = control_byte;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    //while (!(I2C1->CR1 & I2C_CR1_ACK));
    //(void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = highaddr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->DR = lowaddr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    __i2c_stop();
}

uint8_t i2c_curr_read(uint8_t blocksel) {
    uint8_t data;

    __i2c_start();

    uint8_t control_byte = EEPROM_ADDR;

    if (blocksel)
        control_byte |= EEPROM_BLOCK_SEL;

    control_byte |= EEPROM_READ;

    I2C1->DR = control_byte;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_RXNE));
    
    data = (uint8_t)I2C1->DR;

    __i2c_stop();

    return data;
}

uint8_t i2c_random_read(uint8_t blocksel, uint8_t highaddr, uint8_t lowaddr) {
    __i2c_start();

    uint8_t control_byte = EEPROM_ADDR;
    
    if (blocksel)
        control_byte |= EEPROM_BLOCK_SEL;
    
    I2C1->DR = control_byte;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR2;

    I2C1->DR = highaddr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    I2C1->DR = lowaddr;
    while (!(I2C1->SR1 & I2C_SR1_BTF));

    return i2c_curr_read(blocksel);
}



void I2C1_ER_IRQHandler(){
    // error handler
    GPIOD->ODR |= (1 << 14); // red LED
}

void led_init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER &= ~(0xFFU << 24);
    GPIOD->MODER |= (0x55 << 24);
    GPIOD->ODR    = 0x0000;
}

int main() {
    MySystemInit();
    
    set_sysclk_to_168();

    i2c_init_2();

    led_init();

    uint8_t data1 = 0xBE;
    uint8_t data2 = 0xEF;

    i2c_write_byte(0,0x00,0x00,data1);
    i2c_write_byte(0,0x00,0x01,data2);

    uint8_t test1 = i2c_random_read(0,0x00,0x00);
    uint8_t test2 = i2c_curr_read(0);

    if (test1 != data1 || test2 != data2)
        GPIOD->ODR |= (1 << 13);
    
    test2 = i2c_random_read(0,0x00,0x01);
    test1 = i2c_random_read(0,0x00,0x00);

    if (test1 != data1 || test2 != data2)
        GPIOD->ODR |= (1 << 13);

    

    
    while(1) {
        GPIOD->ODR |= (1 << 12);
    }
    return 0;
}
