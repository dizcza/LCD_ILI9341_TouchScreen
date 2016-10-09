/*
 * LCD.h
 *
 */

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#include "registers.h"

#define TFTWIDTH   240
#define TFTHEIGHT  320

#define TFTLCD_DELAY 0xFF

// GPIO to data bus pin connections
// ---- PORT Pin ---     --- Data ----
// GPIOA, GPIO_PIN_9  -> BIT 0
// GPIOC, GPIO_PIN_7  -> BIT 1
// GPIOA, GPIO_PIN_10 -> BIT 2
// GPIOB, GPIO_PIN_3  -> BIT 3
// GPIOB, GPIO_PIN_5  -> BIT 4
// GPIOB, GPIO_PIN_4  -> BIT 5
// GPIOB, GPIO_PIN_10 -> BIT 6
// GPIOA, GPIO_PIN_8  -> BIT 7

#define LCD_CS_PIN  GPIO_PIN_0	// PB0 -> A3 // Chip Select goes to Analog 3
#define LCD_CD_PIN  GPIO_PIN_4	// PA4 -> A2 // Command/Data goes to Analog 2
#define LCD_WR_PIN  GPIO_PIN_1	// PA1 -> A1 // LCD Write goes to Analog 1
#define LCD_RD_PIN  GPIO_PIN_0	// PA0 -> A0 // LCD Read goes to Analog 0
#define LCD_RST_PIN GPIO_PIN_1	// PC1 -> RESET

#define LCD_CS_GPIO_PORT  GPIOB
#define LCD_CS_HIGH()     LCD_CS_GPIO_PORT->BSRR = LCD_CS_PIN
#define LCD_CS_LOW()      LCD_CS_GPIO_PORT->BSRR = (uint32_t)LCD_CS_PIN << 16U

#define LCD_RD_GPIO_PORT  GPIOA
#define LCD_RD_HIGH()     LCD_RD_GPIO_PORT->BSRR = LCD_RD_PIN
#define LCD_RD_LOW()      LCD_RD_GPIO_PORT->BSRR = (uint32_t)LCD_RD_PIN << 16U

#define LCD_WR_GPIO_PORT  GPIOA
#define LCD_WR_HIGH()	  LCD_WR_GPIO_PORT->BSRR = LCD_WR_PIN
#define LCD_WR_LOW()      LCD_WR_GPIO_PORT->BSRR = (uint32_t)LCD_WR_PIN << 16U

#define LCD_CD_GPIO_PORT  GPIOA
#define LCD_CD_HIGH()     LCD_CD_GPIO_PORT->BSRR = LCD_CD_PIN
#define LCD_CD_LOW()      LCD_CD_GPIO_PORT->BSRR = (uint32_t)LCD_CD_PIN << 16U

#define LCD_RST_GPIO_PORT GPIOC
#define LCD_RST_HIGH()    LCD_RST_GPIO_PORT->BSRR = LCD_RST_PIN
#define LCD_RST_LOW()     LCD_RST_GPIO_PORT->BSRR = (uint32_t)LCD_RST_PIN << 16U

#define LCD_WR_STROBE() { LCD_WR_LOW(); delay(4); LCD_WR_HIGH(); delay(4); }

#define swap(a, b) { int16_t t = a; a = b; b = t; }

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define LIGHTGRAY 0xCDB6

/* BEGIN Structures & variables */
int16_t m_cursor_x;
int16_t m_cursor_y;

typedef struct LCD_Point {
	int16_t x, y;
} LCD_Point;

/* END Structures & variables */

void LCD_Init(void);
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);
void LCD_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color);
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void LCD_DrawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t fillcolor);
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void LCD_FillScreen(uint16_t color);
void LCD_Flood(uint16_t color, uint32_t len);
void LCD_Reset(void);
void LCD_SetCursor(unsigned int x, unsigned int y);
void LCD_SetTextSize(uint8_t s);
void LCD_SetTextColor(uint16_t c, uint16_t b);
void LCD_SetTextWrap(uint8_t w);
void LCD_SetAddrWindow(int x1, int y1, int x2, int y2);
void LCD_Printf(const char *fmt, ...);

void LCD_Write8(uint8_t data);
void LCD_WriteRegister8(uint8_t a, uint8_t d);
void LCD_WriteRegister16(uint16_t a, uint16_t d);

uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b);

void LCD_GPIO_Init(void);
void LCD_InitTouchADC(ADC_HandleTypeDef* hadcX, uint32_t ADC_ChannelX, ADC_HandleTypeDef* hadcY, uint32_t ADC_ChannelY);
int8_t LCD_Touch(LCD_Point* p);
void LCD_ClearIfNecessary();

#endif /* __LCD_H */