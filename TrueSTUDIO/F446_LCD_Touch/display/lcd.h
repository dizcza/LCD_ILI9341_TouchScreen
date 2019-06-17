/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 LCD TFT Library for 2.4" MCUfriend shield using 8080 8-bit parallel interface
*	based on Adafruit GFX & Adafruit TFT LCD libraries
*	24 May 2019 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.7 added support for 24-bit BMP
*		- v1.6 added support for SSD1297 (262k only)
*		- v1.5 added printf wipe line/screen defines
*		- v1.4 added support for R61505
*		- v1.3 added support for HX8347D
*		- v1.2 added Courier New font family with Cyrillic (CP1251), created using TheDotFactory font generator
*		- v1.1 added support for UNKNOWN1602, LCD_DrawBMP() & LCD_DrawBMPFromFile()
*		- v1.0 added support for ILI9325, ILI9328, ILI9340, ILI9341, ILI9341_00, R61505V, R61520, S6D0154 chips
*/

// Graphics library by ladyada/adafruit with init code from Rossum
// MIT license

/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).
Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!
Copyright (c) 2013 Adafruit Industries.  All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "registers.h"
#include "Fonts/fonts.h"
#include "stm32f4xx_hal.h"

// Please uncomment one of the lines to select your LCD chip
//#define ILI9325			// works!!
//#define ILI9328			// works!!
//#define ILI9340			// works!!
//#define ILI9340_INV		// works!!
//#define ILI9341			// works!!
#define ILI9341_00		// works!!
//#define R61505			// works!!
//#define R61505V			// works!! (ATTN! Used to be named R61505 in versions below V1.4)
//#define R61520			// works!! // this is a splitfix screen
//#define S6D0154			// works!!
//#define UNKNOWN1602		// works!!
//#define HX8347D			// works!!
//#define HX8347G			// not tested yet
//#define HX8357D			// not tested yet
//#define SSD1297			// works!!

// Please uncomment one of the lines to select your LCD connection mode
#define USE_8080_8BIT
//#define USE_FSMC

// Please uncomment to draw BMP from SD Card
//#define USE_FATFS

// Please uncomment to clear screen when wrapping
//#define WIPE_SCREEN
// Please uncomment to clear line when wrapping
//#define WIPE_LINES

#if !(defined(ILI9325) || defined(ILI9328) \
		|| defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) \
		|| defined(R61505) || defined(R61505V) || defined(R61520) || defined(S6D0154) \
		|| defined (UNKNOWN1602) || defined(HX8347D) || defined(HX8347G) || defined(HX8357D) \
		|| defined (SSD1297))
#error Please select your LCD chip in lcd.h, lines 63-77
#endif

#if defined (USE_FATFS)
#include "ff.h"
#endif

#if !(defined(USE_8080_8BIT) || defined(USE_FSMC))
#error Please select your LCD connection mode in lcd.h, lines 80-81
#endif

#define TFTWIDTH			240
#define TFTHEIGHT			320

#define TFTLCD_DELAY		0xFF

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

// GPIO to control bus pin connections
// ---- PORT Pin ---     --- Signal ----
// GPIOB, GPIO_PIN_0  -> CS
// GPIOA, GPIO_PIN_4  -> CD
// GPIOC, GPIO_PIN_1  -> RST
// GPIOA, GPIO_PIN_0  -> RD
// GPIOA, GPIO_PIN_1  -> WR

#define LCD_CS_GPIO_PORT	GPIOB
#define LCD_CS_PIN			GPIO_PIN_0
#define LCD_CS_IDLE()		LCD_CS_GPIO_PORT->BSRR = LCD_CS_PIN						// CS_HIGH
#define LCD_CS_ACTIVE()		LCD_CS_GPIO_PORT->BSRR = (uint32_t)LCD_CS_PIN << 16U	// CS_LOW

#define LCD_CD_GPIO_PORT	GPIOA
#define LCD_CD_PIN			GPIO_PIN_4
#define LCD_CD_DATA()		LCD_CD_GPIO_PORT->BSRR = LCD_CD_PIN						// CD_HIGH
#define LCD_CD_COMMAND()	LCD_CD_GPIO_PORT->BSRR = (uint32_t)LCD_CD_PIN << 16U	// CD_LOW

#define LCD_RST_GPIO_PORT	GPIOC
#define LCD_RST_PIN			GPIO_PIN_1
#define LCD_RST_IDLE()		LCD_RST_GPIO_PORT->BSRR = LCD_RST_PIN					// RST_HIGH
#define LCD_RST_ACTIVE()	LCD_RST_GPIO_PORT->BSRR = (uint32_t)LCD_RST_PIN << 16U	// RST_LOW

#define LCD_RD_GPIO_PORT	GPIOA
#define LCD_RD_PIN			GPIO_PIN_0
#define LCD_RD_IDLE()		LCD_RD_GPIO_PORT->BSRR = LCD_RD_PIN						// RD_HIGH
#define LCD_RD_ACTIVE()		LCD_RD_GPIO_PORT->BSRR = (uint32_t)LCD_RD_PIN << 16U	// RD_LOW

#define LCD_RD_STROBE() 	do {\
								LCD_RD_ACTIVE();\
								delay(2);\
								LCD_RD_IDLE();\
								delay(2);\
							} while(0)

#define LCD_WR_GPIO_PORT	GPIOA
#define LCD_WR_PIN			GPIO_PIN_1
#define LCD_WR_IDLE()		LCD_WR_GPIO_PORT->BSRR = LCD_WR_PIN						// WR_HIGH
#define LCD_WR_ACTIVE()		LCD_WR_GPIO_PORT->BSRR = (uint32_t)LCD_WR_PIN << 16U	// WR_LOW

#define LCD_WR_STROBE() 	do {\
								LCD_WR_ACTIVE();\
								delay(2);\
								LCD_WR_IDLE();\
								delay(2);\
							} while(0)

#define swap(a, b)			do {\
								int16_t t = a;\
								a = b;\
								b = t;\
							} while(0)

#define	BLACK				0x0000
#define	BLUE				0x001F
#define	RED					0xF800
#define	GREEN				0x07E0
#define CYAN				0x07FF
#define MAGENTA				0xF81F
#define YELLOW				0xFFE0
#define WHITE				0xFFFF
#define LIGHTGRAY			0xCDB6

void LCD_Init(void);
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);
void LCD_Flood(uint16_t color, uint32_t len);
void LCD_FillScreen(uint16_t color);
void LCD_Reset(void);
void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
#if defined(HX8347D) || defined(HX8347G)
void LCD_SetLR(void);
#endif
void LCD_DrawBMP(int16_t xPos, int16_t yPos, const uint8_t *pBmp);
#if defined (USE_FATFS)
void LCD_DrawBMPFromFile(int16_t xPos, int16_t yPos, FIL * pFile);
#endif

void LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color);
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t length, uint16_t color);
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_DrawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
void LCD_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void LCD_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t fontindex);
void LCD_Printf(const char *fmt, ...);

void LCD_SetCursor(uint16_t x, uint16_t y);
void LCD_SetTextSize(uint8_t s);
void LCD_SetTextColor(uint16_t c, uint16_t b);
void LCD_SetTextWrap(uint8_t w);
void LCD_SetRotation(uint8_t x);
uint8_t LCD_GetRotation(void);
int16_t LCD_GetCursorX(void);
int16_t LCD_GetCursorY(void);

uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b);
#endif /* __LCD_H */