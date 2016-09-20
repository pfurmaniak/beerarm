#ifndef HD44780_H
#define HD44780_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"

#include "tm_stm32f4_delay.h"

void LCD_Init();
void LCD_Write(uint8_t x, uint8_t y, char* str);

void LCD_BacklightOn();
void LCD_BacklightOff();

#endif
