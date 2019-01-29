/*
 * lcd_touch.h
 *
 * This library implements touch screen capabilities of TFTLCD, based on ILI9341 driver.
 * To change ILIxxxx driver, refer to lcd.h.
 *
 * LCD Touch pinout:
 *   Y- PB10
 *   Y+ PA4, EXTI4
 *   X- PA1
 *   X+ PA8
 *
 * In idle state, PA4 is in interrupt mode awaiting for touches.
 * You can't draw and await for touches simultaneously.
 * Set LCD_Mode to DRAW to draw anything with LCD, then switch back to TOUCH.
 *
 *  Created on: Oct 29, 2016
 *      Author: dizcza
 */

#ifndef __LCD_TOUCH_H
#define __LCD_TOUCH_H

#define LCD_TOUCH_TOLERANCE_PX 0.5

#include "lcd.h"
#include "stm32f4xx_hal.h"

typedef struct LCD_TouchPoint {
	int16_t x, y;
	uint32_t time;
} LCD_TouchPoint;

typedef enum {
	DRAW = 0,
	TOUCH
} LCD_Mode;

typedef enum {
	IDLE = 0,
	TOUCH_DOWN,
	TOUCH_UP
} LCD_TouchState;

void LCD_Touch_Init(ADC_HandleTypeDef* hadcX, uint32_t ADC_ChannelX, ADC_HandleTypeDef* hadcY, uint32_t ADC_ChannelY);
HAL_StatusTypeDef LCD_SetMode(LCD_Mode mode);

int8_t LCD_Touch_Read();
int8_t LCD_Touch_GetLast(LCD_TouchPoint* p);
int8_t LCD_Touch_DrawLastStroke();
void LCD_Touch_PrintInfo();

void LCD_Touch_OnDown();
void LCD_Touch_OnUp();
LCD_TouchState LCD_Touch_GetState();

#endif /* __LCD_TOUCH_H */
