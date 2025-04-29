/******************************************************************************
 * @author gabman15
 * EE24LC1026 EEPROM Interface
 *
 * Pins:
 * I2C SCL: PB6
 * I2C SDA: PB7
 *****************************************************************************/

//Header Files
//#include "main.h"
#include "led.h"
#include "eeprom.h"

int main(void){
	EEPROM_Init();
    LED_Init();

    /* EEPROM_byteWrite(0x0000, 0x33); */
    
    /* EEPROM_byteWrite(0x0001, 0x44); */
    
    /* uint8_t test1 = EEPROM_byteRead(0x00000); */
    
    /* uint8_t test2 = EEPROM_byteRead(0x00001); */
    /* if(test1 != 0x33 || test2 != 0x44) */
    /*     LED_Orange_On(); */

    uint8_t test[4];
    
    //EEPROM_Write(0x0, test, 4);
    EEPROM_Read(0x0, test, 2);
    /* uint8_t txdata[256]; */
    /* for (int i = 0; i < 256; i++) */
    /*     txdata[i] = i; */

    /* EEPROM_Write(0x0, txdata, 32); */

    /* uint8_t rxdata[256]; */

    /* EEPROM_Read(0x0, rxdata, 32); */

    /* for (int i = 0; i < 32; i++) { */
    /*     if (txdata[i] != rxdata[i]) */
    /*         LED_Orange_On(); */
    /* } */
    
    LED_Green_On();
	while(1); // Dead Loop
}
