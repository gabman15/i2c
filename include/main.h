#ifndef __MAIN_H
#define __MAIN_H

#include "system.h"

// Main PLL = N * (source_clk / M) / P

#define PLL_M 8

#define PLL_P 2

#define PLL_Q 7

#if defined (STM32F407xx)
#define PLL_N 336
#endif

//EEPROM

#define EEPROM_ADDR 0xA0
#define EEPROM_BLOCK_SEL_Pos 1U
#define EEPROM_BLOCK_SEL_Msk (0x1UL << EEPROM_BLOCK_SEL_Pos)
#define EEPROM_BLOCK_SEL EEPROM_BLOCK_SEL_Msk

#define EEPROM_READ_Pos 0
#define EEPROM_READ_Msk (0x1UL << EEPROM_READ_Pos)
#define EEPROM_READ EEPROM_READ_Msk

#endif /* __MAIN_H */
