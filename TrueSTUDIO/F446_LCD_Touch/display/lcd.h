/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 LCD TFT Library for 2.4" MCUfriend shield using 8080 8-bit parallel interface
*	based on Adafruit GFX & Adafruit TFT LCD libraries
*	08 Apr 2020 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.14  optimized lowlevel functions, added lookup table for higher FPS, 
*		         enabled -O2/-Osize library optimization at file level
*		- v1.13  added LCD_SetTextScaled(), LCD_ColorHex() functions
*		- v1.12  added support for ST7781
*		- v1.11  replaced newlib printf with mpaland printf
*		- v1.10  added header function description, refactored code
*		- v1.9   added support for ILI9486
*		- v1.8   added support for ILI9340_INV
*		- v1.7   added support for 24-bit BMP
*		- v1.6   added support for SSD1297 (262k only)
*		- v1.5   added printf wipe line/screen defines
*		- v1.4   added support for R61505
*		- v1.3   added support for HX8347D
*		- v1.2   added Courier New font family with Cyrillic (CP1251), created using TheDotFactory font generator
*		- v1.1   added support for UNKNOWN1602, LCD_DrawBMP() & LCD_DrawBMPFromFile()
*		- v1.0   added support for ILI9325, ILI9328, ILI9340, ILI9341, ILI9341_00, R61505V, R61520, S6D0154 chips
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

// Please uncomment one of the lines to select your LCD chip
//#define ILI9325
//#define ILI9328
//#define ILI9340
//#define ILI9340_INV
//#define ILI9341
#define ILI9341_00
//#define ILI9486
//#define R61505
//#define R61505V			// ATTN! Used to be named R61505 in versions below V1.4
//#define R61520			// note: this is a splitfix screen
//#define S6D0154
//#define UNKNOWN1602
//#define HX8347D
//#define SSD1297
//#define ST7781			// almost works :)

// TODO: #define HX8347G			// not tested yet
// TODO: #define HX8357D			// not tested yet

// Uncomment to clear screen when wrapping
//#define WIPE_SCREEN

// Uncomment to clear line when wrapping
//#define WIPE_LINES

// Uncomment to add FatFs support for drawing BMPs from SD Card
//#define USE_FATFS

// Uncomment to use mpaland printf (faster and lighter) instead of libc printf
// Source: https://github.com/mpaland/printf/
#define USE_MPALAND_PRINTF

// Uncomment to use lookup table for data output (average x2 increase in FPS, +3kB RAM)
// FillScreen(0x051F) @ 180 MHz with lookup table    -O0: 25.5 FPS, -O2: 54.5 FPS
// FillScreen(0x051F) @ 180 MHz without lookup table -O0: 16 FPS,   -O2: 24 FPS
#define USE_LOOKUP

#if !(defined(ILI9325) || defined(ILI9328) || defined(ILI9340) || defined(ILI9340_INV) \
		|| defined(ILI9341) || defined(ILI9341_00) || defined(ILI9486) \
		|| defined(R61505) || defined(R61505V) || defined(R61520) || defined(S6D0154) \
		|| defined (UNKNOWN1602) || defined(HX8347D) || defined(HX8347G) || defined(HX8357D) \
		|| defined (SSD1297) || defined(ST7781))
#error Error! Please select your LCD chip in lcd.h, lines 67-81
#endif

#if defined (USE_FATFS)
#include "ff.h"
#endif

#if defined (USE_MPALAND_PRINTF)
#include "printf/printf.h"
#else
#include <stdio.h>
#endif

#define TFTWIDTH			240
#define TFTHEIGHT			320

#define	BLACK				0x0000
#define	BLUE				0x001F
#define	RED					0xF800
#define	GREEN				0x07E0
#define CYAN				0x07FF
#define MAGENTA				0xF81F
#define YELLOW				0xFFE0
#define WHITE				0xFFFF
#define LIGHTGRAY			0xCDB6

/* =========================================================================== */
/* ============================ BASIC FUNCTIONS ============================== */
/* =========================================================================== */

/**
 * \brief LCD Initialization
 *
 * \param
 *
 * \return void
 */
void LCD_Init(void);

/**
 * \brief Draws a point at the specified coordinates
 *
 * \param x		x-Coordinate
 * \param y		y-Coordinate
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);

/**
 * \brief Floods screen with pixels
 *
 * \param color	Color
 * \param len	Length
 *
 * \return void
 */
void LCD_Flood(uint16_t color, uint32_t len);

/**
 * \brief Fills the screen with the specified color
 *
 * \param color	Color
 *
 * \return void
 */
void LCD_FillScreen(uint16_t color);

/**
 * \brief Resets the Display
 *
 * \param
 *
 * \return void
 */
void LCD_Reset(void);

/**
 * \brief Set display rotation
 *
 * \param x		Rotation:	0 - 0 deg (portrait)
 * 							1 - 90 deg CW (album)
 * 							2 - 180 deg CW (reversed portrait)
 * 							3 - 90 deg CCW (reversed album)
 *
 * \return void
 */
void LCD_SetRotation(uint8_t x);

/**
 * \brief Get display rotation
 *
 * \param
 *
 * \return uint8_t rotation
 */
uint8_t LCD_GetRotation(void);

/**
 * \brief Sets window address
 *
 * \param x1
 * \param y1
 * \param x2
 * \param y2
 *
 * \return void
 */
void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * \brief  Draws a BMP picture loaded in the STM32 MCU internal memory.
 *
 * \param xPos		Bmp X position in the LCD
 * \param yPos		Bmp Y position in the LCD
 * \param pBmp		Pointer to Bmp picture address
 * \return void
 */
void LCD_DrawBMP(int16_t xPos, int16_t yPos, const uint8_t *pBmp);

/**
 * \brief  Draws a bitmap picture from FatFs file.
 *
 * \param  xPos: Bmp X position in the LCD
 * \param  yPos: Bmp Y position in the LCD
 * \param  pFile: Pointer to FIL object with bmp picture
 * \retval None
 */
#if defined (USE_FATFS)
void LCD_DrawBMPFromFile(int16_t xPos, int16_t yPos, FIL * pFile);
#endif

/* =========================================================================== */
/* ============================ GFX FUNCTIONS ================================ */
/* =========================================================================== */

/**
 * \brief Draws a line connecting the two points specified by the coordinate pairs
 *
 * \param x0	The x-coordinate of the first point
 * \param y0	The y-coordinate of the first point
 * \param x1	The x-coordinate of the second point
 * \param y1	The y-coordinate of the second point.
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

/**
 * \brief Draws a horizontal line
 *
 * \param x			The x-coordinate of the first point
 * \param y			The y-coordinate of the first point
 * \param length	Length of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color);

/**
 * \brief Draws a vertical line
 *
 * \param x		The x-coordinate of the first point
 * \param y		The y-coordinate of the first point
 * \param h		High of the line
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t length, uint16_t color);

/**
 * \brief Draws a rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * \brief Draws a filled rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param color			Color
 *
 * \return void
 */
void LCD_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * \brief Draws an circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/**
 * \brief Helper function drawing rounded corners
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param color			Color
 *
 * \return void
 */
void LCD_DrawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);

/**
 * \brief Draws a filled circle defined by a pair of coordinates and radius
 *
 * \param x0		The x-coordinate
 * \param y0		The y-coordinate
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

/**
 * \brief Helper function to draw a filled circle
 *
 * \param x0			The x-coordinate
 * \param y0			The y-coordinate
 * \param r				Radius
 * \param cornername	Corner (1, 2, 3, 4)
 * \param delta			Delta
 * \param color			Color
 *
 * \return void
 */
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);

/**
 * \brief Draws a triangle specified by the coordinate pairs.
 *
 * \param x0		The x-coordinate of the first point
 * \param y0		The y-coordinate of the first point
 * \param x1		The x-coordinate of the second point
 * \param y1		The y-coordinate of the second point
 * \param x2		The x-coordinate of the third point
 * \param y2		The y-coordinate of the third point
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/**
 * \brief Draws a filled triangle specified by the coordinate pairs.
 *
 * \param x0		The x-coordinate of the first point
 * \param y0		The y-coordinate of the first point
 * \param x1		The x-coordinate of the second point
 * \param y1		The y-coordinate of the second point
 * \param x2		The x-coordinate of the third point
 * \param y2		The y-coordinate of the third point
 * \param color		Color
 *
 * \return void
 */
void LCD_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

/**
 * \brief Draws a rectangle with rounded corners specified by a coordinate pair, a width, and a height.
 *
 * \param x			The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y			The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w			Width of the rectangle to draw
 * \param h			Height of the rectangle to draw
 * \param r			Radius
 * \param color		Color
 *
 * \return void
 */
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

/**
 * \brief Draws a filled rounded rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param r				Radius
 * \param fillcolor		Color
 *
 * \return void
 */
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

/**
 * \brief Calculate RGB565 (16 bit) Color from RGB888 (24 bit)
 *
 * \param r	Red
 * \param g	Green
 * \param b	Blue
 *
 * \return uint16_t	RGB565 (16 bit) Color
 */
uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b);

/**
 * \brief Calculate RGB565 (16 bit) Color from RGB888 Hex string
 *
 * \param hexcolor	RGB888 Hex string starting with '#'
 *
 * \return uint16_t	RGB565 (16 bit) Color
 */
uint16_t LCD_ColorHex(char * hexcolor);

/* =========================================================================== */
/* ============================ TEXT FUNCTIONS =============================== */
/* =========================================================================== */

/**
 * \brief Draws a character at the specified coordinates
 *
 * \param x			The x-coordinate
 * \param y			The y-coordinate
 * \param c			Character
 * \param color		Character color
 * \param bg		Background color
 * \param size		Character Size
 *
 * \return void
 */
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t fontindex);

/**
 * \brief Print the specified Text
 *
 * \param fmt	Format text
 * \param
 *
 * \return void
 */
void LCD_Printf(const char *fmt, ...);

/**
 * \brief Sets the cursor coordinates
 *
 * \param x		The x-coordinate
 * \param y		The y-coordinate
 *
 * \return void
 */
void LCD_SetCursor(uint16_t x, uint16_t y);

/**
 * \brief Gets the cursor x-coordinate
 *
 * \param
 *
 * \return int16_t x-coordinate
 */
int16_t LCD_GetCursorX(void);

/**
 * \brief Gets the cursor Y-coordinate
 *
 * \param
 *
 * \return int16_t y-coordinate
 */
int16_t LCD_GetCursorY(void);

/**
 * \brief Sets the text size
 *
 * \param s	Size
 *
 * \return void
 */
void LCD_SetTextSize(uint8_t s);

/**
 * \brief Gets the text size
 *
 * \param
 *
 * \return uint8_t size
 */
uint8_t LCD_GetTextSize(void);

/**
 * \brief Sets the text color
 *
 * \param c		Text color
 * \param b		Background color
 *
 * \return void
 */
void LCD_SetTextColor(uint16_t c, uint16_t b);

/**
 * \brief Set Text wrap
 *
 * \param w
 *
 * \return void
 */
void LCD_SetTextWrap(uint8_t w);

/**
 * \brief Sets the text scaling
 *
 * \param s		Scaling
 *
 * \return void
 */
void LCD_SetTextScaled(uint8_t s);

#endif /* __LCD_H */
