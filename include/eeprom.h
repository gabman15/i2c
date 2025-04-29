//#include "stm32f4xx.h"
#include "i2c.h"

#define EEPROM_ADDRESS 0xA0

#define EEPROM_MAX_ADDRESS 0x1FFFF

#define EEPROM_SIZE 0x2000

#define EEPROM_PAGE_SIZE 0x80

void EEPROM_Init();

void EEPROM_byteWrite(uint32_t address, uint8_t data);

uint8_t EEPROM_byteRead(uint32_t address);

void EEPROM_Write(uint32_t address, uint8_t *databuf, uint32_t len);

void EEPROM_Read(uint32_t address, uint8_t *databuf, uint32_t len);
