/******************************************************************************
 * This is the code for interfacing a 16x2 LCD with a I2C module using the
 * I2C protocol, and printing "HELLO" on the LCD Screen.
 *
 * Configurations are as follows:
 * PB6 - I2C1_SCL
 * PB7 - I2C1_SDA
 * (See Page 58 of the 203 Page user stm32f407vg user manual)
 *
 * For the I2C Module (PC58574T) to LCD the connections are as follows:
 * P0 - RS
 * P1 - R/W
 * P2 - E
 * P3 - Back Light
 * P4 - D4
 * P5 - D5
 * P6 - D6
 * P7 - D7
 * So, the data byte that we send through the I2C module will be mapped to the
 * LCD based on the above configuration. This will be helpful for understanding
 * how the data and commands are sent to the LCD in the LCD_Write_Data and
 * LCD_Write_Cmd function.
 *****************************************************************************/

//Header Files
#include <main.h>
#define ARM_MATH_CM4

//Register Definitions for 1602A I2C LCD

// Variable for clearing ADDR bit
uint8_t clc;

//User-defined Function Declarations
void GPIO_Init(void);
void I2C_Init(void);
void I2C_Start(void);
void I2C_Write(uint8_t var);
void I2C_Send_Addr(uint8_t Addr);
void I2C_Stop(void);
void TIM4_ms_Delay(uint16_t delay);

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

//Definitions of the User-defined Functions
void GPIO_Init(){
	//Enable GPIOB clock
	RCC->AHB1ENR |= 1UL<<1;

	// Configuring PB6 and PB7 in Alternate function
	GPIOB->MODER |= ( (2UL<<(6*2)) | (2UL<<(7*2)) );

	// Selecting PB6 and PB7 as Pull up pins
	GPIOB->PUPDR |= ( (1UL<<(6*2)) | (1UL<<(7*2)));

	// Setting PB6 and PB7 as open drain
	GPIOB->OTYPER |= ( (1UL<<6) | (1UL<<7) );

	// Setting PB6 and PB7 at high speed
	GPIOB->OSPEEDR |= ( (2UL<<(6*2)) | (2UL<<(7*2)) );

	// Selecting the Alternate function (AF4)
	GPIOB->AFR[0] |= ( (4UL<<(6*4)) | (4UL)<<(7*4));
}

void I2C_Init(){
	// Enable I2C1 clock
	RCC->APB1ENR |= 1UL<<21;

	// Reset I2C
	I2C1->CR1 |= 1UL<<15;
	I2C1->CR1 &= ~(1UL<<15);

	// Set I2C clock at 16MHz
	I2C1->CR2 |= 16UL<<0;

	// Needs to be set high by software for I2C
	I2C1->OAR1 |= 1UL<<14;

	// Set SCl at 100KHz
	I2C1->CCR |= 0x50UL<<0;

	// Configure rise time as 1000ns
	I2C1->TRISE |= 17UL<<0;

	// Enable I2C1
	I2C1->CR1 |= 1UL<<0;
}

void I2C_Start(){
	// Enable the acknowledgment ACK bit
	I2C1->CR1 |= 1UL<<10;

	// Set the START bit to Start Communication
	I2C1->CR1 |= 1UL<<8;

	// SB bit is set when START condition is generated
	// So, it is polled to make sure communication has started
	while(!(I2C1->SR1 & I2C_SR1_SB)){}
}

void I2C_Write(uint8_t var){
	// Polling the TxE bit in the I2C_SR1 register to see if
	// data register is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE)){}

	// Put the data to be written in the I2C_DR register
	I2C1->DR = var;

	// Poll the BTF (Byte Transfer Finished) bit in the I2C_SR1 register to confirm
	// Byte transfer
	while(!(I2C1->SR1 & I2C_SR1_BTF)){}
}

uint8_t I2C_Read(){

    while (!(I2C1->SR1 & I2C_SR1_RXNE));

    return (uint8_t)I2C1->DR;
}

void I2C_Send_Addr(uint8_t Addr){
	//Put the address to be sent into the I2C_DR register
	I2C1->DR = Addr;

	// ADDR bit is polled in the I2C_SR1 register for end of
	// address transmission
	while(!(I2C1->SR1 & I2C_SR1_ADDR)){}

	// Variable that will read I2C_SR1 and I2C_SR2
	// for clearing the ADDR bit
	clc = (I2C1->SR1 | I2C1->SR2);
}

void I2C_Stop(){
	// Stop Communication after current byte transfer
	I2C1->CR1 |= 1UL<<9;
    while (!(I2C1->SR2 & I2C_SR2_BUSY));
}

/*void LCD_Write_Cmd(uint8_t Device_Addr,uint8_t Slave_Reg_Addr, uint8_t data){
	uint8_t d_h, d_l, d1, d2, d3, d4;
	d_h = data & 0xF0;
	d_l = (data << 4) & 0xF0;

	// Set EN = 1 and RS = 0 and R/W =0 for writing to
	// Instruction Register and send the Upper 4 bits
	d1 = d_h | 0x0C;
	d2 = d_h | 0x08; // Set EN = 0 and RS = 0 and set Back Light on

	// Set EN = 1 and RS = 0 and R/W = 0
	// for Instruction Register and send the Lower 4 bits
	d3 = d_l | 0x0C;
	d4 = d_l | 0x08; // Set EN = 0 and RS = 0 and set Back Light on

	I2C_Start();
	I2C_Send_Addr(Device_Addr);
	I2C_Write(Slave_Reg_Addr);
	I2C_Write(d1);
	TIM4_ms_Delay(2); // Wait for 2 ms for the command to take action
	I2C_Write(d2);
	I2C_Write(d3);
	TIM4_ms_Delay(2); // Wait for 2 ms for the command to take action
	I2C_Write(d4);
	I2C_Stop();
}

void LCD_Write_Data(uint8_t Device_Addr, uint8_t Slave_Reg_Addr, uint8_t data){
	uint8_t d_h, d_l, d1, d2, d3, d4;
	d_h = data & 0xF0;
	d_l = (data << 4) & 0xF0;

	// Set EN = 1 and RS = 1 and R/W = 0 for writing to
	// Data Register and send the Upper 4 bits
	d1 = d_h | 0x0D;
	d2 = d_h | 0x09; // Set EN = 0 and RS = 1 and set Back Light on

	// Set EN = 1 and RS = 1 and R/W = 0 for writing to
	// Data Register and send the Lower 4 bits
	d3 = d_l | 0x0D;
	d4 = d_l | 0x09; // Set EN = 0 and RS = 1 and set Back Light on

	I2C_Start();
	I2C_Send_Addr(Device_Addr);
	I2C_Write(Slave_Reg_Addr);
	I2C_Write(d1);
	TIM4_ms_Delay(2); // Wait for 2 ms for the data to be written
	I2C_Write(d2);
	I2C_Write(d3);
	TIM4_ms_Delay(2); // Wait for 2 ms for the data to be written
	I2C_Write(d4);
	I2C_Stop();
    }*/


void TIM4_ms_Delay(uint16_t delay){
	RCC->APB1ENR |= 1<<2; //Start the clock for the timer peripheral
	TIM4->PSC = 168000-1; //Setting the clock frequency to 1kHz.
	TIM4->ARR = (delay); // Total period of the timer
	TIM4->CNT = 0;
	TIM4->CR1 |= 1; //Start the Timer
	while(!(TIM4->SR & TIM_SR_UIF)){} //Polling the update interrupt flag
	TIM4->SR &= ~(0x0001); //Reset the update interrupt flag
}

void led_init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    GPIOD->MODER &= ~(0xFFU << 24);
    GPIOD->MODER |= (0x55 << 24);
    GPIOD->ODR    = 0x0000;
}

uint8_t I2C1_byteRead(char slaveaddress, char highaddress, uint8_t lowaddress)
{
	volatile int tmp;

	//Wait until I2C1 is no longer busy
	while((I2C1->SR2) & (1U << 1));

	//Generate start
	I2C1->CR1 |= (1 << 8);

	//Wait until start flag is set
	while(!((I2C1->SR1) & (1U << 0)));

	//Transmit slave address + Write
	I2C1->DR = slaveaddress << 1;

	//Wait until addr flag is set
 	while(!((I2C1->SR1) & (1U << 1)));

	//Clear addr flag
	tmp= I2C1->SR2;

	//tentative
	while(!((I2C1->SR1) & (1U << 7)));

	//Send high memory address
	I2C1->DR = highaddress;

	//Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));

    //Send low memory address
	I2C1->DR = lowaddress;

	//Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));

	//Generate restart condition
	I2C1->CR1 |= (1 << 8);

	//Wait until start flag is set
	while(!(I2C1->SR1 & (1 << 0)));

	//Transmit slave address + Read
	I2C1->DR = slaveaddress << 1 | 1;

	//Wait until addr flag is set
	while(!(I2C1->SR1 & (1U << 1)));

	//Disable Acknowledge (ACK)
	I2C1->CR1 &= ~(1 << 10);

	//Clear addr flag
	tmp= I2C1->SR2;

	//Generate stop 
	I2C1->CR1 |= (1 << 9);

	//Wait until RXNE flag is set
	while(!(I2C1->SR1 & (1U << 6)));

	//Read data from DR
	return (uint8_t)I2C1->DR;

}

uint8_t I2C1_poll(uint8_t slaveaddress) {
    //Wait until I2C1 is no longer busy
	while((I2C1->SR2) & (1U << 1));

    //Enable Ack
    I2C1->CR1 |= (1 << 10);
    
    //Generate start
	I2C1->CR1 |= (1 << 8);

	//Wait until start flag is set
	while(!((I2C1->SR1) & (1U << 0)));

	//Transmit slave address + Write
	I2C1->DR = slaveaddress << 1;

	//Wait until addr flag is set
 	if(!((I2C1->CR1) & (1U << 10)))
        return 0;
    return 1;

}

void I2C1_byteWrite(uint8_t slaveaddress, uint8_t highaddress, uint8_t lowaddress, uint8_t data) {
    volatile int tmp;
    //Wait until I2C1 is no longer busy
	while((I2C1->SR2) & (1U << 1));

    //Enable Ack
    I2C1->CR1 |= (1 << 10);
    
    //Generate start
	I2C1->CR1 |= (1 << 8);

	//Wait until start flag is set
	while(!((I2C1->SR1) & (1U << 0)));

	//Transmit slave address + Write
	I2C1->DR = slaveaddress << 1;

	//Wait until addr flag is set
 	while(!((I2C1->SR1) & (1U << 1)));

	//Clear addr flag
	tmp= I2C1->SR2;

	//tentative
	while(!((I2C1->SR1) & (1U << 7)));

	//Send high memory address
	I2C1->DR = highaddress;

	//Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));

    //Send low memory address
	I2C1->DR = lowaddress;

	//Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));

    I2C1->DR = data;

	//Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));

    while(!(I2C1->SR1 & I2C_SR1_BTF));
    
    //Clear addr flag
	//tmp= I2C1->SR2;

	//Generate stop 
	I2C1->CR1 |= (1 << 9);

    //Wait until transmitter is empty
	while(!((I2C1->SR1) & (1U << 7)));
}



int main(void){
	GPIO_Init();
	I2C_Init();
    led_init();
    System_Clock_Init();

    /*I2C_Start();
    I2C_Send_Addr(EEPROM_ADDR);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Write(0x00);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Write(0x00);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Write(0x66);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Stop();
    
    //POLL
    do {
        I2C1->CR1 |= 1UL<<8;
        I2C_Send_Addr(EEPROM_ADDR);    
    } while (!(I2C1->CR1 & I2C_CR1_ACK));
    
    I2C_Start();
    I2C_Send_Addr(EEPROM_ADDR);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Write(0x00);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Write(0x00);
    while (!(I2C1->CR1 & I2C_CR1_ACK));
    I2C_Stop();
    I2C_Start();
    I2C_Send_Addr(EEPROM_ADDR | 1);
    while (!(I2C1->CR1 & I2C_CR1_ACK));*/

    I2C1_byteWrite(EEPROM_ADDR >> 1, 0x00, 0x00, 0x11);
    TIM4_ms_Delay(1000000);
    I2C1_byteWrite(EEPROM_ADDR >> 1, 0x00 ,0x01, 0x22);
    /*TIM4_ms_Delay(5);
    uint8_t test1 = I2C1_byteRead(EEPROM_ADDR >> 1,0x00,0x00);
    TIM4_ms_Delay(5);
    uint8_t test2 = I2C1_byteRead(EEPROM_ADDR >> 1,0x00,0x01);
    if(test1 != 0x11 || test2 != 0x22)
        GPIOD->ODR |= (1 << 13);
    */
	while(1){ // Dead Loop
        GPIOD->ODR |= (1 << 12);
	}
}
