/*
 * lcd_touch.h
 *
 *  Created on: Oct 29, 2016
 *      Author: dizczaw7
 */

#ifndef __LCD_TOUCH_H
#define __LCD_TOUCH_H

#include "lcd.h"
#include "stm32f4xx_hal.h"

/*
 * LCD Touch pinout:
 *
 *  Y+ PB10
 *  Y- PA4
 *  X+ PA1
 *  X- PA8
 */

void LCD_InitTouchADC(ADC_HandleTypeDef* hadcX, uint32_t ADC_ChannelX, ADC_HandleTypeDef* hadcY, uint32_t ADC_ChannelY);
int8_t LCD_Touch(LCD_Point* p);

#endif /* __LCD_TOUCH_H */
