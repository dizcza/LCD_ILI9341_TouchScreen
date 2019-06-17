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

#include "lcd.h"
#include "stm32f4xx_hal.h"

typedef enum {
	LCD_MODE_DRAW = 0,
	LCD_MODE_TOUCH
} LCD_Mode;

typedef enum {
	LCD_TOUCH_IDLE = 0,  // idle; no touch is made yet
	LCD_TOUCH_DOWN,
	LCD_TOUCH_MOVE,
	LCD_TOUCH_UP
} LCD_TouchState;

typedef enum {
	LCD_TOUCH_READ_SUCCESS = 0,
	LCD_TOUCH_READ_NOT_INITIALIZED,  // user did not call LCD_Touch_Init()
	LCD_TOUCH_READ_NO_TOUCH,  // idle
	LCD_TOUCH_READ_OUTSIDE    // ADC value is outside of the acceptable range
} LCD_TouchReadState;

typedef struct LCD_TouchPoint {
	int16_t x, y;
	uint32_t tick;  // touch time in ms
	LCD_TouchState state;
} LCD_TouchPoint;


void LCD_Touch_Init(ADC_HandleTypeDef* hadcX, uint32_t ADC_ChannelX, ADC_HandleTypeDef* hadcY, uint32_t ADC_ChannelY);
HAL_StatusTypeDef LCD_SetMode(LCD_Mode mode);

LCD_TouchReadState LCD_Touch_Read(LCD_TouchPoint* p);
void LCD_Touch_OnDown();
void LCD_Touch_OnUp();
LCD_TouchState LCD_Touch_GetState();

/* LCD Touch Draw */
void LCD_Touch_Draw_ConnectLastPoint(const LCD_TouchPoint* p);
void LCD_Touch_Draw_PrintInfo();
void LCD_Touch_Draw_OnUp();

#endif /* __LCD_TOUCH_H */
