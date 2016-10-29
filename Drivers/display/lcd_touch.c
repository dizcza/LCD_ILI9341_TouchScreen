/*
 * lcd_touch.c
 *
 *  Created on: Oct 29, 2016
 *      Author: dizczaw7
 */

#include <stdlib.h>
#include "lcd_touch.h"

static ADC_HandleTypeDef* hadcX = NULL;
static ADC_HandleTypeDef* hadcY = NULL;
static uint32_t ADC_ChannelX;
static uint32_t ADC_ChannelY;

#define ADC_NO_TOUCH_X (4095 - 100)
#define TOUCH_ADC_X_MAX 3600
#define TOUCH_ADC_X_MIN 500
#define TOUCH_ADC_Y_MIN 300
#define TOUCH_ADC_Y_MAX 3780
#define clamp(x,l,u) (x < l ? l : (x > u ? u : x))


static float adc_norm_x(uint32_t x) {
	return (x - TOUCH_ADC_X_MIN) * 1.0 / (TOUCH_ADC_X_MAX - TOUCH_ADC_X_MIN);
}

static float adc_norm_y(uint32_t y) {
	return (y - TOUCH_ADC_Y_MIN) * 1.0 / (TOUCH_ADC_Y_MAX - TOUCH_ADC_Y_MIN);
}


static uint32_t ADC_GetValue(ADC_HandleTypeDef* hadc, uint32_t channel){
    ADC_ChannelConfTypeDef sConfig;

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(hadc, &sConfig);

    // start conversion
    HAL_ADC_Start(hadc);

	// wait until finish
    HAL_ADC_PollForConversion(hadc, 100);

    uint32_t value = HAL_ADC_GetValue(hadc);

    HAL_ADC_Stop(hadc);

    return value;
}

static void GPIO_SetPinMode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_PinMode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_PinMode;
	GPIO_InitStruct.Pull = GPIO_PinMode == GPIO_MODE_INPUT ? GPIO_PULLUP : GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


static void ADC_GPIOA_init(uint16_t GPIO_Pin) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


static uint32_t touchX() {
	GPIO_SetPinMode(GPIOA, GPIO_PIN_1, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOB, GPIO_PIN_10, GPIO_MODE_INPUT);
	ADC_GPIOA_init(GPIO_PIN_4);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
	//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);

	return ADC_GetValue(hadcX, ADC_ChannelX);
}

static uint32_t touchY() {
	GPIO_SetPinMode(GPIOB, GPIO_PIN_10, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_8, GPIO_MODE_INPUT);
	ADC_GPIOA_init(GPIO_PIN_1);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0);

	return ADC_GetValue(hadcY, ADC_ChannelY);
}


static void LCD_relax() {
	LCD_Write8(0x00);
}


void LCD_InitTouchADC(ADC_HandleTypeDef* aHadcX, uint32_t aADC_ChannelX,
					  ADC_HandleTypeDef* aHadcY, uint32_t aADC_ChannelY) {
	hadcX = aHadcX;
	hadcY = aHadcY;
	ADC_ChannelX = aADC_ChannelX;
	ADC_ChannelY = aADC_ChannelY;
}


int8_t LCD_Touch(LCD_Point* p) {
	if (hadcX == NULL || hadcY == NULL) return -1;
	LCD_relax();
	uint32_t x = touchX();

	if (x > ADC_NO_TOUCH_X) {
		LCD_GPIO_Init();
		return 1;
	}

	uint32_t y = touchY();

	p->x = (int16_t) ((1 - clamp(adc_norm_x(x), 0.0f, 1.0f)) * TFTWIDTH);
	p->y = (int16_t) ((1 - clamp(adc_norm_y(y), 0.0f, 1.0f)) * TFTHEIGHT);

	LCD_GPIO_Init();
	return 0;
}
