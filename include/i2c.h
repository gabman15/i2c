#include "stm32f407xx.h"
#include "delay.h"

void I2C_Init();

void I2C_byteWrite(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t data);

uint8_t I2C_byteRead(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress);

void I2C_Write(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t *databuf, uint16_t len);

void I2C_Read(uint8_t device_address, uint8_t highaddress, uint8_t lowaddress, uint8_t *databuf, uint16_t len);
