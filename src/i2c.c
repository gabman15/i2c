#include "i2c.h"
int I2CRX (uint8_t slave_addr, uint8_t number_of_bytes, uint8_t* bytes_received) {
	/*
	 *
	 * Expects a certain number of bytes over the bus from the slave
	 * Each byte is automatically ACKed by the master, except the last one - NACK will go HIGH!
	 * Data input is an array pointer - pass the function either the array, or the address of the array
	 * STOP is generated automatically due to AUTOEND
	 *
	 */
													//we reset the CR2 register and rebuild it completely to avoid an address being stuck in there

		//2)We set the slave address and the addressing mode
		//I2C1->CR2 = (slave_addr);											//we write the slave address to the SADD register
																				//Note: we are in 7-bit address mode, as it was set above in the I2C config
		//I2C1->CR2 &= ~(1<<11);													//7 bits

		//3)We set NBYTES
		//I2C1->CR2 |= (number_of_bytes << 16);

		//4)AUTOEND with write as direction, no RELOAD
		//I2C1->CR2 |= (1 << 25);
		//I2C1->CR2 &= ~(1 << 24);


		//7)We wait for a reply
		//DELAY_ms(1);											//latency wait

		uint8_t reply = 0;

		if (I2C1->SR1 & I2C_SR1_AF) {									//if the AF bit is 1, it means that we had a NO ACKNOWLEDGE from the slave
			reply = 0;
		} else {																//if NACK is 0, the slave has been ACKed
			while (number_of_bytes) {
				while (!(I2C1->SR1 & I2C_SR1_RXNE));									//we wait for the RXNE flag to go HIGH, indicating that we have something in the RXDN register
				*bytes_received = I2C1->DR;									//we dereference the pointer, thus we can give it a value
				bytes_received++;												//this is technically an address. A pointer is technically a memory address. Here we step through the array at the address.
				number_of_bytes--;
			}
			reply = 1;
		}

		//8)We reset the bus and all the registers in ISR
        I2C1->CR1 |= I2C_CR1_SWRST;
        I2C1->CR1 &= ~(I2C_CR1_SWRST);
		DELAY_ms(1);
		return reply;
}

static void __i2c_start() {
    
    while(I2C1->SR2 & I2C_SR2_BUSY);

    //Generate Start
	I2C1->CR1 |= I2C_CR1_START;

	//Wait until Start flag is set
	while(!(I2C1->SR1 & I2C_SR1_SB));
}

static void __i2c_stop() {
    //Generate stop 
	I2C1->CR1 |= I2C_CR1_STOP;
}

static void __i2c_write_address(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress) {
    __i2c_start();
    
	//Transmit device address + Write
	I2C1->DR = device_address;

	//Wait until addr flag is set
 	while(!(I2C1->SR1 & I2C_SR1_ADDR));

	//Clear addr flag
	(void)I2C1->SR2;

	//Make sure Data register is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));

	//Send high memory address
	I2C1->DR = highaddress;

	//Wait until transmitter is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));

    //Send low memory address
	I2C1->DR = lowaddress;

	//Wait until transmitter is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));
}

static void __i2c_write_byte(uint8_t data) {
    //Send data
    I2C1->DR = data;

    //Wait until transmitter is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));

    //Wait until ByteField sent
    while(!(I2C1->SR1 & I2C_SR1_BTF));
}

static uint8_t __i2c_read_byte() {
    //Wait until RXNE flag is set
	while(!(I2C1->SR1 & I2C_SR1_RXNE));

	//Read data from DR
	return (uint8_t)I2C1->DR;
}

//Initializes I2C GPIO Pins
void GPIO_Init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Enable GPIOB clock (where i2c pins are)

    //Set mode of PB7 and PB7 to alternate functions
    GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7); // Clear mode of PB8 and PB9
    GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;

    // Set output type of PB6 and PB7 to open drain
    GPIOB->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;

    /* // Set speed of PB6 and PB7 to High Speed */
    /* GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6_0 | GPIO_OSPEEDR_OSPEED6_1) | \ */
    /*     (GPIO_OSPEEDR_OSPEED7_0 | GPIO_OSPEEDR_OSPEED7_1); */

    // Set PB6 and PB7 to have internal pull up resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7); // Clear pull up mode for PB6 and PB7
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0;
    
    // SET PB6 and PB7 to AF4 (corresponding to I2C SCL and SDA respectively)
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7); // Clear alternate function on PB6 and PB7
    GPIOB->AFR[0] |= (4U << GPIO_AFRL_AFSEL6_Pos) | (4U << GPIO_AFRL_AFSEL7_Pos);
}

void I2C_Init(){
    GPIO_Init();

    DELAY_Init();
    
	// Enable I2C1 clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	// Reset I2C
	I2C1->CR1 |= I2C_CR1_SWRST;
	I2C1->CR1 &= ~(I2C_CR1_SWRST);

	// Set I2C clock at 16MHz
	I2C1->CR2 |= (16UL<<I2C_CR2_FREQ_Pos);

	// Needs to be set high by software for I2C
	I2C1->OAR1 |= (1UL<<14);

	// Set SCl at 100KHz
	I2C1->CCR |= (0x50UL<<I2C_CCR_CCR_Pos);

	// Configure rise time as 1000ns
	I2C1->TRISE |= (17UL<<0);

	// Enable I2C1
	I2C1->CR1 |= I2C_CR1_PE;
}

uint8_t I2C_byteRead(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress)
{
    __i2c_write_address(device_address, highaddress, lowaddress);

    //Restart i2c

    //Generate Start
	I2C1->CR1 |= I2C_CR1_START;

	//Wait until Start flag is set
	while(!(I2C1->SR1 & I2C_SR1_SB));

	//Transmit device address + Read
	I2C1->DR = device_address | 1;

    //Wait until addr flag is set
 	while(!(I2C1->SR1 & I2C_SR1_ADDR));
    
	//Disable Acknowledge (ACK)
	I2C1->CR1 &= ~(I2C_CR1_ACK);

	//Clear addr flag
	(void)I2C1->SR2;

    //Wait until RXNE flag is set
	while(!(I2C1->SR1 & I2C_SR1_RXNE));

    //Stop i2c
	I2C1->CR1 |= I2C_CR1_STOP;

	//Read data from DR
	return (uint8_t)I2C1->DR;
}



void I2C_byteWrite(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t data) {
    __i2c_write_address(device_address, highaddress, lowaddress);

    __i2c_write_byte(data);
    
    __i2c_stop();

    //Wait until transmitter is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));
    
    DELAY_ms(5);
}

void I2C_Write(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t *databuf, uint16_t len) {
    __i2c_write_address(device_address, highaddress, lowaddress);

    for (int i = 0; i < len; i++) {
        __i2c_write_byte(databuf[i]);
    }

    __i2c_stop();

    //Wait until transmitter is empty
	while(!(I2C1->SR1 & I2C_SR1_TXE));
    
    DELAY_ms(5);
}

void I2C_Read(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t *databuf, uint16_t len) {
    /* if (len == 1) { */
    /*     databuf[0] = I2C_byteRead(device_address, highaddress, lowaddress); */
    /*     return; */
    /* } */

    

    
    __i2c_write_address(device_address, highaddress, lowaddress);

    //Restart i2c
    
    //Generate Start
	I2C1->CR1 |= I2C_CR1_START;

	//Wait until Start flag is set
	while(!(I2C1->SR1 & I2C_SR1_SB));

	//Transmit device address + Read
	I2C1->DR = device_address | 1;

    //Wait until addr flag is set
 	while(!(I2C1->SR1 & I2C_SR1_ADDR));

    //(void)I2C1->SR2;
    //I2CRX(device_address, len, databuf);
    if (len == 2) {
        //Disable Acknowledge (ACK)
        I2C1->CR1 &= ~(I2C_CR1_ACK);

        //Set POS flag
        I2C1->CR1 |= I2C_CR1_POS;

        //Clear addr flag
        (void)I2C1->SR2;

        while(!(I2C1->SR1 & I2C_SR1_BTF));

        //Stop i2c
        I2C1->CR1 |= I2C_CR1_STOP;

        databuf[0] = I2C1->DR;
        databuf[1] = I2C1->DR;
    }

    for (int i = 0; i < len-2; i++) {
        while(!(I2C1->SR1 & I2C_SR1_BTF));
        
        //Disable Acknowledge (ACK)
        I2C1->CR1 &= ~(I2C_CR1_ACK);
        
        databuf[i] = I2C1->DR;
    }

    //while(!(I2C1->SR1 & I2C_SR1_BTF));
        
    __i2c_stop();
        
    databuf[len-2] = I2C1->DR;
    databuf[len-1] = I2C1->DR;
}


