/*
 * lcd_touch.h
 *
 * LCD Touch pinout:
 *   Y+ PB10
 *   Y- PA4
 *   X+ PA1
 *   X- PA8
 *
 *  Created on: Oct 29, 2016
 *      Author: dizcza
 */

#ifndef __LCD_TOUCH_H
#define __LCD_TOUCH_H

#include "lcd.h"
#include "stm32f4xx_hal.h"

typedef enum {
	TOUCH = 0,
	DRAW = 1
} LCD_Mode;

typedef enum {
	IDLE = 0,
	TOUCH_DOWN = 1,
	TOUCH_UP = 2
} LCD_TouchState;

void LCD_InitTouchADC(ADC_HandleTypeDef* hadcX, uint32_t ADC_ChannelX, ADC_HandleTypeDef* hadcY, uint32_t ADC_ChannelY);
int8_t LCD_ReadTouch(LCD_Point* p);
HAL_StatusTypeDef LCD_SetMode(LCD_Mode mode);

void LCD_OnTouchDown();
void LCD_OnTouchUp();
LCD_TouchState LCD_GetTouchState();

#endif /* __LCD_TOUCH_H */
