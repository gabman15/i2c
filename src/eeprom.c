#include "eeprom.h"
#include "i2c.h"

/* Private Functions */

void EEPROM_Page_Write(uint32_t address, uint8_t *databuf, uint8_t len) {
    if (len == 0)
        return;
    uint8_t blocksel = address >> 16;
    uint8_t highaddress = (address & 0xFF00) >> 8;
    uint8_t lowaddress = address & 0xFF;
    uint8_t device_address = EEPROM_ADDRESS | blocksel;

    I2C_Write(device_address, highaddress, lowaddress, databuf, len);
}

static uint32_t __bytes_till_block(uint32_t address) {
    return 0x10000-(address & 0xFFFF);
}

static uint8_t __get_max_writeable_bytes(uint32_t address) {
    uint32_t num = __bytes_till_block(address);
    if (num > EEPROM_PAGE_SIZE)
        return EEPROM_PAGE_SIZE;
    return (uint8_t) num;
}

/* Public Functions */

void EEPROM_byteWrite(uint32_t address, uint8_t data) {
    if (address > EEPROM_MAX_ADDRESS)
        return;
    uint8_t blocksel = address >> 16;
    uint8_t highaddress = address & 0xFF00;
    uint8_t lowaddress = address & 0xFF;
    uint8_t device_address = EEPROM_ADDRESS | (blocksel << 1);
    I2C_byteWrite(device_address, highaddress, lowaddress, data);
}

uint8_t EEPROM_byteRead(uint32_t address) {
    if (address > EEPROM_MAX_ADDRESS)
        return 0;
    uint8_t blocksel = address >> 16;
    uint8_t highaddress = address & 0xFF00;
    uint8_t lowaddress = address & 0xFF;
    uint8_t device_address = EEPROM_ADDRESS | (blocksel << 1);
    return I2C_byteRead(device_address, highaddress, lowaddress);
}

void EEPROM_Write(uint32_t address, uint8_t *databuf, uint32_t len) {
    if (address > EEPROM_MAX_ADDRESS || len > EEPROM_SIZE || (EEPROM_SIZE - address) < len)
        return;

    while (len > EEPROM_PAGE_SIZE) {
        uint8_t num_bytes = __get_max_writeable_bytes(address);
        EEPROM_Page_Write(address, databuf, num_bytes);
        address += num_bytes;
        databuf += num_bytes;
        len -= num_bytes;
    }
    uint8_t num_bytes = __get_max_writeable_bytes(address);

    if (num_bytes >= len) {
        EEPROM_Page_Write(address, databuf, len);
        return;
    }

    EEPROM_Page_Write(address, databuf, num_bytes);
    address += num_bytes;
    databuf += num_bytes;
    len -= num_bytes;
    EEPROM_Page_Write(address, databuf, len);
}

void EEPROM_Read(uint32_t address, uint8_t *databuf, uint32_t len) {
    if (address > EEPROM_MAX_ADDRESS || len > EEPROM_SIZE || (EEPROM_SIZE - address) < len)
        return;

    uint8_t blocksel = address >> 16;
    uint8_t highaddress = address & 0xFF00;
    uint8_t lowaddress = address & 0xFF;
    uint8_t device_address = EEPROM_ADDRESS | (blocksel << 1);

    uint32_t num = __bytes_till_block(address);

    if (blocksel || num >= len) {
        I2C_Read(device_address, highaddress, lowaddress, databuf, len);
        return;
    }

    I2C_Read(device_address, highaddress, lowaddress, databuf, num);
    address += num;
    databuf += num;
    len -= num;
    I2C_Read(device_address, highaddress, lowaddress, databuf, len);
}

void EEPROM_Init() {
    I2C_Init();
}
