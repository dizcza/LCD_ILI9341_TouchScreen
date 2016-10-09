#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "lcd.h"
#include "glcdfont.h"

static int16_t m_width;
static int16_t m_height;

static uint16_t m_textcolor;
static uint16_t m_textbgcolor;
static uint8_t m_textsize;
static uint8_t m_wrap;

static ADC_HandleTypeDef* hadcX = NULL;
static ADC_HandleTypeDef* hadcY = NULL;
static uint32_t ADC_ChannelX;
static uint32_t ADC_ChannelY;

#define ADC_NO_TOUCH_X (4095 - 200)
#define TOUCH_ADC_X_MAX 3600
#define TOUCH_ADC_X_MIN 500
#define TOUCH_ADC_Y_MIN 300
#define TOUCH_ADC_Y_MAX 3780
#define clamp(x,l,u) (x < l ? l : (x > u ? u : x))

void delay(unsigned int t) {
	for (; t > 0; t--) {
		__asm("nop");
	}
}

/**
 * \brief GPIO Initialization
 *
 * \param
 *
 * \return void
 */
void LCD_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE()
	;
	__GPIOA_CLK_ENABLE()
	;
	__GPIOB_CLK_ENABLE()
	;

	/*Configure GPIO pins : PC1 PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA2 PA5 PA8
	 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_8 | GPIO_PIN_9
			| GPIO_PIN_10 | GPIO_PIN_1 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB10 PB4 PB5 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_10 | GPIO_PIN_4
			| GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * \brief LCD Initialization
 *
 * \param
 *
 * \return void
 */
void LCD_Init(void) {
	m_width = TFTWIDTH;
	m_height = TFTHEIGHT;
	m_cursor_y = m_cursor_x = 0;
	m_textsize = 4;
	m_textcolor = m_textbgcolor = 0xFFFF;
	m_wrap = 1;

	LCD_GPIO_Init();

	LCD_Reset();
	HAL_Delay(50);

	LCD_CS_LOW();

	LCD_WriteRegister8(ILI9341_SOFTRESET, 0);
	HAL_Delay(50);
	LCD_WriteRegister8(ILI9341_DISPLAYOFF, 0);

	LCD_WriteRegister8(ILI9341_POWERCONTROL1, 0x23);
	LCD_WriteRegister8(ILI9341_POWERCONTROL2, 0x10);
	LCD_WriteRegister16(ILI9341_VCOMCONTROL1, 0x2B2B);
	LCD_WriteRegister8(ILI9341_VCOMCONTROL2, 0xC0);

	LCD_WriteRegister8(ILI9341_MEMCONTROL,
				ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR); // change rotation here.

	LCD_WriteRegister8(ILI9341_PIXELFORMAT, 0x55);
	LCD_WriteRegister16(ILI9341_FRAMECONTROL, 0x001B);

	LCD_WriteRegister8(ILI9341_ENTRYMODE, 0x07);
	LCD_WriteRegister8(ILI9341_INVERTON, 0);
	LCD_WriteRegister8(ILI9341_SLEEPOUT, 0);
	HAL_Delay(150);
	LCD_WriteRegister8(ILI9341_DISPLAYON, 0);
	HAL_Delay(100);

	LCD_SetAddrWindow(0, 0, TFTWIDTH - 1, TFTHEIGHT - 1);
	LCD_CS_HIGH();
}

/**
 * \brief Calucalte 16Bit-RGB
 *
 * \param r	Red
 * \param g	Green
 * \param b	Blue
 *
 * \return uint16_t	16Bit-RGB
 */
uint16_t LCD_Color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * \brief Draws a point at the specified coordinates
 *
 * \param x		x-Coordinate
 * \param y		y-Coordinate
 * \param color	Color
 *
 * \return void
 */
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color) {
	// Clip
	if ((x < 0) || (y < 0) || (x >= TFTWIDTH) || (y >= TFTHEIGHT))
		return;

	LCD_CS_LOW();

	LCD_SetAddrWindow(x, y, TFTWIDTH - 1, TFTHEIGHT - 1);

	LCD_CS_LOW();
	uint8_t hi = color >> 8, lo = color;

	LCD_CD_LOW();
	//LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(0x2C); // Write data to GRAM

	LCD_CD_HIGH();
	LCD_Write8(hi);
	LCD_Write8(lo);

	LCD_CS_HIGH();
}

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
void LCD_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
		uint16_t color) {
	// Bresenham's algorithm - thx wikpedia

	int16_t steep = abs(y2 - y1) > abs(x2 - x1);
	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	int16_t dx, dy;
	dx = x2 - x1;
	dy = abs(y2 - y1);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y1 < y2) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x1 <= x2; x1++) {
		if (steep) {
			LCD_DrawPixel(y1, x1, color);
		} else {
			LCD_DrawPixel(x1, y1, color);
		}
		err -= dy;
		if (err < 0) {
			y1 += ystep;
			err += dx;
		}
	}
}

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
void LCD_DrawFastHLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int16_t x2;
#ifdef LCD_SPLIT_FIX
	if (y >= 160) y = 1 + (y % 160) * 2;
	else y = y * 2;
#endif
	// Initial off-screen clipping
	if ((length <= 0) || (y < 0) || (y >= m_height) || (x >= m_width) || ((x2 =
			(x + length - 1)) < 0))
		return;

	if (x < 0)   // Clip left
			{
		length += x;
		x = 0;
	}

	if (x2 >= m_width)   // Clip right
			{
		x2 = m_width - 1;
		length = x2 - x + 1;
	}

	LCD_SetAddrWindow(x, y, x2, y);
	LCD_Flood(color, length);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);

}

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
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	// Update in subclasses if desired!
	LCD_DrawLine(x, y, x, y + h - 1, color);
}

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
void LCD_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	LCD_DrawFastHLine(x, y, w, color);
	LCD_DrawFastHLine(x, y + h - 1, w, color);
	LCD_DrawFastVLine(x, y, h, color);
	LCD_DrawFastVLine(x + w - 1, y, h, color);
}

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
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
		uint16_t color) {
	// smarter version
	LCD_DrawFastHLine(x + r, y, w - 2 * r, color); // Top
	LCD_DrawFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
	LCD_DrawFastVLine(x, y + r, h - 2 * r, color); // Left
	LCD_DrawFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
	// draw four corners
	LCD_DrawCircleHelper(x + r, y + r, r, 1, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	LCD_DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	LCD_DrawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

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
void LCD_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
		uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			LCD_DrawPixel(x0 + x, y0 + y, color);
			LCD_DrawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			LCD_DrawPixel(x0 + x, y0 - y, color);
			LCD_DrawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			LCD_DrawPixel(x0 - y, y0 + x, color);
			LCD_DrawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			LCD_DrawPixel(x0 - y, y0 - x, color);
			LCD_DrawPixel(x0 - x, y0 - y, color);
		}
	}
}

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
void LCD_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_DrawPixel(x0, y0 + r, color);
	LCD_DrawPixel(x0, y0 - r, color);
	LCD_DrawPixel(x0 + r, y0, color);
	LCD_DrawPixel(x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		LCD_DrawPixel(x0 + x, y0 + y, color);
		LCD_DrawPixel(x0 - x, y0 + y, color);
		LCD_DrawPixel(x0 + x, y0 - y, color);
		LCD_DrawPixel(x0 - x, y0 - y, color);
		LCD_DrawPixel(x0 + y, y0 + x, color);
		LCD_DrawPixel(x0 - y, y0 + x, color);
		LCD_DrawPixel(x0 + y, y0 - x, color);
		LCD_DrawPixel(x0 - y, y0 - x, color);
	}
}

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
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,
		uint16_t bg, uint8_t size) {
	if ((x >= m_width) || // Clip right
			(y >= m_height) || // Clip bottom
			((x + 6 * size - 1) < 0) || // Clip left
			((y + 8 * size - 1) < 0))   // Clip top
		return;

	for (int8_t i = 0; i < 6; i++) {
		uint8_t line;
		if (i == 5) {
			line = 0x0;
		} else {
			line = font[c * 5 + i];   //pgm_read_byte(font+(c*5)+i);
			for (int8_t j = 0; j < 8; j++) {
				if (line & 0x1) {
					if (size == 1)   // default size
							{
						LCD_DrawPixel(x + i, y + j, color);
					} else      // big size
					{
						LCD_FillRect(x + (i * size), y + (j * size), size, size,
								color);
					}
				} else if (bg != color) {
					if (size == 1)   // default size
							{
						LCD_DrawPixel(x + i, y + j, bg);
					} else      // big size
					{
						LCD_FillRect(x + i * size, y + j * size, size, size,
								bg);
					}
				}
				line >>= 1;
			}
		}
	}
}

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
void LCD_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	LCD_DrawFastVLine(x0, y0 - r, 2 * r + 1, color);
	LCD_FillCircleHelper(x0, y0, r, 3, 0, color);
}

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
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,
		int16_t delta, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			LCD_DrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			LCD_DrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
			LCD_DrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

/**
 * \brief Draws a filled rectangle specified by a coordinate pair, a width, and a height.
 *
 * \param x				The x-coordinate of the upper-left corner of the rectangle to draw
 * \param y				The y-coordinate of the upper-left corner of the rectangle to draw
 * \param w				Width of the rectangle to draw
 * \param h				Height of the rectangle to draw
 * \param fillcolor		Color
 *
 * \return void
 */
void LCD_FillRect(int16_t x, int16_t y1, int16_t w, int16_t h,
		uint16_t fillcolor) {
	int16_t x2, y2;

	// Initial off-screen clipping
	if ((w <= 0) || (h <= 0) || (x >= m_width) || (y1 >= m_height)
			|| ((x2 = x + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
		return;
	if (x < 0)   // Clip left
			{
		w += x;
		x = 0;
	}
	if (y1 < 0)   // Clip top
			{
		h += y1;
		y1 = 0;
	}
	if (x2 >= m_width)   // Clip right
			{
		x2 = m_width - 1;
		w = x2 - x + 1;
	}
	if (y2 >= m_height)   // Clip bottom
			{
		y2 = m_height - 1;
		h = y2 - y1 + 1;
	}

	LCD_SetAddrWindow(x, y1, x2, y2);
	LCD_Flood(fillcolor, (uint32_t) w * (uint32_t) h);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

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
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
		uint16_t color) {
	// smarter version
	LCD_FillRect(x + r, y, w - 2 * r, h, color);

	// draw four corners
	LCD_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	LCD_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

/**
 * \brief Fills the screen with the specified color
 *
 * \param color	Color
 *
 * \return void
 */
void LCD_FillScreen(uint16_t color) {
	LCD_CS_LOW();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
	LCD_Flood(color, (long) TFTWIDTH * (long) TFTHEIGHT);
}

/**
 * \brief Flood
 *
 * \param color	Color
 * \param len	Length
 *
 * \return void
 */
void LCD_Flood(uint16_t color, uint32_t len) {
	uint16_t blocks;
	uint8_t i, hi = color >> 8, lo = color;

	LCD_CS_LOW();
	LCD_CD_LOW();
	//LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(0x2C); // Write data to GRAM

	// Write first pixel normally, decrement counter by 1
	LCD_CD_HIGH();
	LCD_Write8(hi);
	LCD_Write8(lo);
	len--;

	blocks = (uint16_t) (len / 64); // 64 pixels/block
	if (hi == lo) {
		// High and low bytes are identical.  Leave prior data
		// on the port(s) and just toggle the write strobe.
		while (blocks--) {
			i = 16; // 64 pixels/block / 4 pixels/pass
			do {
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				; // 2 bytes/pixel
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				;
				LCD_WR_STROBE()
				; // x 4 pixels
			} while (--i);
		}
		// Fill any remaining pixels (1 to 64)
		for (i = (uint8_t) len & 63; i--;) {
			LCD_WR_STROBE()
			;
			LCD_WR_STROBE()
			;
		}
	} else {
		while (blocks--) {
			i = 16; // 64 pixels/block / 4 pixels/pass
			do {
				LCD_Write8(hi);
				LCD_Write8(lo);
				LCD_Write8(hi);
				LCD_Write8(lo);
				LCD_Write8(hi);
				LCD_Write8(lo);
				LCD_Write8(hi);
				LCD_Write8(lo);
			} while (--i);
		}
		for (i = (uint8_t) len & 63; i--;) {
			LCD_Write8(hi);
			LCD_Write8(lo);
		}
	}
	LCD_CS_HIGH();
}

/**
 * \brief Print the specified Text
 *
 * \param fmt	Format text
 * \param
 *
 * \return void
 */
void LCD_Printf(const char *fmt, ...) {
	static char buf[256];
	char *p;
	va_list lst;

	va_start(lst, fmt);
	vsprintf(buf, fmt, lst);
	va_end(lst);

	p = buf;
	while (*p) {
		if (*p == '\n') {
			m_cursor_y += m_textsize * 8;
			m_cursor_x = 0;
		} else if (*p == '\r') {
			// skip em
		} else {
			LCD_DrawChar(m_cursor_x, m_cursor_y, *p, m_textcolor, m_textbgcolor,
					m_textsize);
			m_cursor_x += m_textsize * 6;
			if (m_wrap && (m_cursor_x > (m_width - m_textsize * 6))) {
				m_cursor_y += m_textsize * 8;
				m_cursor_x = 0;
			}
		}
		p++;

		if (m_cursor_y >= 320) {
			m_cursor_y = 0;
		}
	}
}

/**
 * \brief Resets the Display
 *
 * \param
 *
 * \return void
 */
void LCD_Reset(void) {
	LCD_CS_HIGH();
	LCD_WR_HIGH();
	LCD_RD_HIGH();

	LCD_RST_LOW();
	HAL_Delay(100);
	LCD_RST_HIGH();

	// Data transfer sync
	LCD_CS_LOW();

	LCD_CD_LOW();
	LCD_Write8(0x00);
	for (uint8_t i = 0; i < 3; i++)
		LCD_WR_STROBE()
	; // Three extra 0x00s
	LCD_CS_HIGH();
}

/**
 * \brief Sets the cursor coordinates
 *
 * \param x		The x-coordinate
 * \param y		The y-coordinate
 *
 * \return void
 */
void LCD_SetCursor(unsigned int x, unsigned int y) {
	m_cursor_x = x;
	m_cursor_y = y;
}

/**
 * \brief Sets the text size
 *
 * \param s	Size
 *
 * \return void
 */
void LCD_SetTextSize(uint8_t s) {
	m_textsize = (s > 0) ? s : 1;
}

/**
 * \brief Sets the text color
 *
 * \param c		Text color
 * \param b		Background color
 *
 * \return void
 */
void LCD_SetTextColor(uint16_t c, uint16_t b) {
	m_textcolor = c;
	m_textbgcolor = b;
}

/**
 * \brief Set Text wrap
 *
 * \param w
 *
 * \return void
 */
void LCD_SetTextWrap(uint8_t w) {
	m_wrap = w;
}

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
void LCD_SetAddrWindow(int x1, int y1, int x2, int y2) {

	LCD_CS_LOW();

	uint32_t t;

	t = x1;
	t <<= 16;
	t |= x2;

	LCD_CD_LOW();
	LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(ILI9341_COLADDRSET);
	LCD_CD_HIGH();
	LCD_Write8(t >> 24);
	LCD_Write8(t >> 16);
	LCD_Write8(t >> 8);
	LCD_Write8(t);

	t = y1;
	t <<= 16;
	t |= y2;
	LCD_CD_LOW();
	//LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(ILI9341_PAGEADDRSET);

	LCD_CD_HIGH();
	LCD_Write8(t >> 24);
	LCD_Write8(t >> 16);
	LCD_Write8(t >> 8);
	LCD_Write8(t);

	LCD_CS_HIGH();
}

/**
 * \brief Writes 8-Bit data
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
void LCD_Write8(uint8_t data) {
	// ------ PORT -----     --- Data ----
	// GPIOA, GPIO_PIN_9  -> BIT 0 -> 0x01
	// GPIOC, GPIO_PIN_7  -> BIT 1 -> 0x02
	// GPIOA, GPIO_PIN_10 -> BIT 2 -> 0x04
	// GPIOB, GPIO_PIN_3  -> BIT 3 -> 0x08
	// GPIOB, GPIO_PIN_5  -> BIT 4 -> 0x10
	// GPIOB, GPIO_PIN_4  -> BIT 5 -> 0x20
	// GPIOB, GPIO_PIN_10 -> BIT 6 -> 0x40
	// GPIOA, GPIO_PIN_8  -> BIT 7 -> 0x80

	GPIOA->ODR = (GPIOA->ODR & 0xF8FF) | ((data & 0x01) << 9)
			| ((data & 0x04) << 8) | ((data & 0x80) << 1);
	GPIOB->ODR = (GPIOB->ODR & 0xFBC7) | (data & 0x08) | ((data & 0x10) << 1)
			| ((data & 0x20) >> 1) | ((data & 0x40) << 4);
	GPIOC->ODR = (GPIOC->ODR & 0xFF7F) | ((data & 0x02) << 6);

	LCD_WR_STROBE()
	;
}

/**
 * \brief Writes 8-Bit register
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
void LCD_WriteRegister8(uint8_t a, uint8_t d) {
	LCD_CD_LOW();
	LCD_Write8(a);
	LCD_CD_HIGH();
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit register
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
void LCD_WriteRegister16(uint16_t a, uint16_t d) {
	uint8_t hi, lo;
	hi = (a) >> 8;
	lo = (a);
	LCD_CD_LOW();
	LCD_Write8(hi);
	LCD_Write8(lo);
	hi = (d) >> 8;
	lo = (d);
	LCD_CD_HIGH();
	LCD_Write8(hi);
	LCD_Write8(lo);
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


void LCD_InitTouchADC(ADC_HandleTypeDef* aHadcX, uint32_t aADC_ChannelX,
					  ADC_HandleTypeDef* aHadcY, uint32_t aADC_ChannelY) {
	hadcX = aHadcX;
	hadcY = aHadcY;
	ADC_ChannelX = aADC_ChannelX;
	ADC_ChannelY = aADC_ChannelY;
}

/*
Y+ PB10
Y- PA4
X+ PA1
X- PA8
 */


float adc_norm_x(uint32_t x) {
	return (x - TOUCH_ADC_X_MIN) * 1.0 / (TOUCH_ADC_X_MAX - TOUCH_ADC_X_MIN);
}

float adc_norm_y(uint32_t y) {
	return (y - TOUCH_ADC_Y_MIN) * 1.0 / (TOUCH_ADC_Y_MAX - TOUCH_ADC_Y_MIN);
}

int8_t LCD_Touch(LCD_Point* p) {
	if (hadcX == NULL || hadcY == NULL) return -1;
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

void LCD_ClearIfNecessary() {
	if (m_cursor_y >= TFTHEIGHT - m_textsize * 8) {
	  LCD_SetCursor(0, 0);
	  LCD_FillScreen(BLACK);
	}
}
