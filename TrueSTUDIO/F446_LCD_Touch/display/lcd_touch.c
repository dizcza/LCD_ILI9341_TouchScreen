/*
 * lcd_touch.c
 *
 *  Created on: Oct 29, 2016
 *      Author: Danylo Ulianych
 *
 *
 * An example of an interrupt handler that
 * should be put in stm32f4xx_it.c:
 *
 *	void EXTI4_IRQHandler(void) {
 *		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) {
 *			LCD_Touch_OnDown();
 *		} else {
 *			LCD_Touch_OnUp();
 *		}
 *		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
 *	}
 *
 */

#include <stdlib.h>
#include "lcd_touch.h"

#define ADC_NO_TOUCH_X_OUTSIDE (4095 - 100)
#define TOUCH_ADC_X_MAX 3600
#define TOUCH_ADC_X_MIN 500
#define TOUCH_ADC_Y_MIN 300
#define TOUCH_ADC_Y_MAX 3780

static const float ADC_UNIT_PX_X = 1.0 / (TOUCH_ADC_X_MAX - TOUCH_ADC_X_MIN);
static const float ADC_UNIT_PX_Y = 1.0 / (TOUCH_ADC_Y_MAX - TOUCH_ADC_Y_MIN);

static ADC_HandleTypeDef* hadcX = NULL;
static ADC_HandleTypeDef* hadcY = NULL;
static uint32_t ADC_ChannelX;
static uint32_t ADC_ChannelY;
static LCD_TouchState m_touch_state = LCD_TOUCH_IDLE;
static LCD_TouchPoint* m_last_point_ref = NULL;

static float fclamp(float x, float l, float u) {
	return x < l ? l : (x > u ? u : x);
}

static float adc_norm_x(uint32_t x) {
	return (x - TOUCH_ADC_X_MIN) * ADC_UNIT_PX_X;
}

static float adc_norm_y(uint32_t y) {
	return (y - TOUCH_ADC_Y_MIN) * ADC_UNIT_PX_Y;
}

static uint32_t ADC_GetValue(ADC_HandleTypeDef* hadc, uint32_t channel) {
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

static void GPIO_SetPinMode(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,
		uint32_t GPIO_PinMode) {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_PinMode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
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

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

	return ADC_GetValue(hadcX, ADC_ChannelX);
}

static uint32_t touchY() {
	HAL_NVIC_DisableIRQ(EXTI4_IRQn);
	GPIO_SetPinMode(GPIOB, GPIO_PIN_10, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP);
	GPIO_SetPinMode(GPIOA, GPIO_PIN_8, GPIO_MODE_INPUT);
	ADC_GPIOA_init(GPIO_PIN_1);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	uint32_t adc_y = ADC_GetValue(hadcY, ADC_ChannelY);

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	return adc_y;
}

/**
 * Saves ADC handles references to measure touch screen positions.
 */
void LCD_Touch_Init(ADC_HandleTypeDef* aHadcX, uint32_t aADC_ChannelX,
		ADC_HandleTypeDef* aHadcY, uint32_t aADC_ChannelY) {
	hadcX = aHadcX;
	hadcY = aHadcY;
	ADC_ChannelX = aADC_ChannelX;
	ADC_ChannelY = aADC_ChannelY;
}

static void GPIO_DrawMode() {
	/* GPIO Ports Clock Enable */
	__GPIOA_CLK_ENABLE()
	;
	__GPIOB_CLK_ENABLE()
	;
	HAL_NVIC_DisableIRQ(EXTI4_IRQn);

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	/*Configure GPIO pins: PA1 PA4 PA8 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_8;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO data pin PB10 */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


// TOUCH mode GPIO setup
static void GPIO_InterruptMode() {
	__GPIOA_CLK_ENABLE()
	;

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	/* X- PA1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

	/* X+ PA8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Y- PB10 */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Y+ PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
}


/**
 * Set LCD's mode to either DRAW or TOUCH.
 *
 * Set LCD_Mode to DRAW to draw or print text on LCD,
 * then switch back to TOUCH, if you want to receive touches.
 */
HAL_StatusTypeDef LCD_SetMode(LCD_Mode mode) {
	switch (mode) {
	case LCD_MODE_TOUCH:
		GPIO_InterruptMode();
		return HAL_OK;

	case LCD_MODE_DRAW:
		GPIO_DrawMode();
		return HAL_OK;

	default:
		return HAL_ERROR;
	}
}

/*
 * Reads raw touch x- and y-positions and, if successful,
 * stores them in the LCD_TouchPoint point.
 */
LCD_TouchReadState LCD_Touch_Read(LCD_TouchPoint* p) {
	if (hadcX == NULL || hadcY == NULL) {
		return LCD_TOUCH_READ_NOT_INITIALIZED;
	}
	if (m_touch_state == LCD_TOUCH_IDLE) {
		return LCD_TOUCH_READ_NO_TOUCH;
	}
	uint32_t x = touchX();

	if (x > ADC_NO_TOUCH_X_OUTSIDE) {
		return LCD_TOUCH_READ_OUTSIDE;
	}

	uint32_t y = touchY();

	p->x = (int16_t) ((1 - fclamp(adc_norm_x(x), 0.0f, 1.0f)) * TFTWIDTH);
	p->y = (int16_t) ((1 - fclamp(adc_norm_y(y), 0.0f, 1.0f)) * TFTHEIGHT);
	p->tick = HAL_GetTick();
	p->state = m_touch_state;

	m_last_point_ref = p;
	m_touch_state = LCD_TOUCH_MOVE;

	return LCD_TOUCH_READ_SUCCESS;
}


/*
 * Indicates the start of a touch.
 * Should be called from EXTIx_IRQHandler interrupt only.
 */
void LCD_Touch_OnDown() {
	if (m_touch_state == LCD_TOUCH_IDLE) {
		m_touch_state = LCD_TOUCH_DOWN;
	}
}


/*
 * Indicates the finish of a touch.
 * Should be called from EXTIx_IRQHandler interrupt only.
 */
void LCD_Touch_OnUp() {
	m_touch_state = LCD_TOUCH_IDLE;

	if (m_last_point_ref != NULL) {
		// Mark the last read touch point as TOUCH_UP.
		// Note that it changes the point state that the user specified
		// as an argument to LCD_Touch_Read function.
		m_last_point_ref->state = LCD_TOUCH_UP;
	}
	LCD_Touch_Draw_OnUp();
}



/*
 * Returns the current touch state.
 */
LCD_TouchState LCD_Touch_GetState() {
	return m_touch_state;
}
