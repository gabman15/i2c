#include "ee24.h"
#include "stm32f4_discovery.h"

EE24_HandleTypeDef ee_24lc1026;

I2C_HandleTypeDef HI2c;
#define I2Cx                             I2C1
#define I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1
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

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable GPIO Clocks #################################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = I2Cx_SCL_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
  
  /*##-3- Enable I2C peripheral Clock ########################################*/
  /* Enable I2C1 clock */
  I2Cx_CLK_ENABLE(); 
}


int main() {
    HAL_Init();
    SystemClock_Config();
    
    HI2c.Instance = I2Cx;
    HI2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    HI2c.Init.ClockSpeed = 100000;
    HI2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    HI2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    HI2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    HI2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    HI2c.Init.OwnAddress1     = 0x0;
    HI2c.Init.OwnAddress2     = 0x0;

    HAL_I2C_MspInit(&HI2c);
    HAL_I2C_Init(&HI2c);
    
    while((EE24_Init(&ee_24lc1026, &HI2c, 0xA0)) != true);
    
    //volatile HAL_StatusTypeDef status = HAL_I2C_Init(&HI2c);
    
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED6);
            
    uint16_t addr = 0xFFF0;
    uint8_t txdata[256];
    for (int i = 0; i < 256; i++) {
        txdata[i] = i;
    }
    uint8_t rxdata[256];

    //status = HAL_I2C_Mem_Read(&HI2c, 0xA0, (uint16_t)addr, I2C_MEMADD_SIZE_16BIT, rxdata, 1, 1000);
    

    
    if(EE24_Write(&ee_24lc1026, addr, (uint8_t *)txdata, 256, 10000) == false)
        BSP_LED_On(LED5);
    
    if(EE24_Read(&ee_24lc1026, addr, (uint8_t *)rxdata, 256, 10000) == false)
        BSP_LED_On(LED6);
    for (int i = 0; i < 32; i++) {
        if (rxdata[i] != txdata[i])
            BSP_LED_On(LED3);
    }
    BSP_LED_On(LED4);
    while(1){}
}
