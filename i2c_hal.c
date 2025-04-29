#include "i2c_hal.h"
#include <stdlib.h>

#define MASTER_BOARD

#define EEPROM_ADDRESS 0xA0

I2C_HandleTypeDef I2cHandle;

static void SystemClock_Config(void);
static void Error_Handler(void);

int main() {

    HAL_Init();

    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);

    SystemClock_Config();

    I2cHandle.Instance = I2Cx;

    I2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.ClockSpeed      = 100000;

    HAL_I2C_Init(&
    
    uint8_t addr[] = {0x00,0x00};
    
    uint8_t *txdata = malloc(sizeof(uint8_t) * 32);

    uint8_t *rxdata = malloc(sizeof(uint8_t) * 32);

    txdata[0] = 0x00;
    txdata[1] = 0x00;
    txdata[2] = 0xFE;

    //Send address
    while(HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)EEPROM_ADDRESS, txdata, 3, 10000)!= HAL_OK) {
        /* Error_Handler() function is called when Timeout error occurs.
           When Acknowledge failure occurs (Slave don't acknowledge it's address)
           Master restarts communication */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
            {
                free(txdata);
                free(rxdata);
                Error_Handler();
            }
    }

    

    BSP_LED_On(LED4);

    //Send address
    while(HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)EEPROM_ADDRESS, addr, 2, 10000)!= HAL_OK) {
        /* Error_Handler() function is called when Timeout error occurs.
           When Acknowledge failure occurs (Slave don't acknowledge it's address)
           Master restarts communication */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
            {
                free(txdata);
                free(rxdata);
                Error_Handler();
            }
    }

    //Receive Data
    while(HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)EEPROM_ADDRESS, rxdata, 1, 10000)!= HAL_OK) {
        /* Error_Handler() function is called when Timeout error occurs.
           When Acknowledge failure occurs (Slave don't acknowledge it's address)
           Master restarts communication */
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
            {
                free(txdata);
                free(rxdata);
                Error_Handler();
            }
    }
    if(rxdata[0] != txdata[0])
        BSP_LED_On(LED5);

    BSP_LED_On(LED6);

    free(txdata);
    free(rxdata);
    return 0;

}



static void Error_Handler(void)
{
  /* Turn LED5 on */
  BSP_LED_On(LED5);
  while(1)
  {
  }
}


static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /* STM32F405x/407x/415x/417x Revision Z and upper devices: prefetch is supported  */
  if (HAL_GetREVID() >= 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}
