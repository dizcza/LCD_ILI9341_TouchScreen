/**
*	TechMaker
*	https://techmaker.ua
*
*	STM32 LCD TFT Library for 2.4" MCUfriend shield using 8080 8-bit parallel interface
*	based on Adafruit GFX & Adafruit TFT LCD libraries
*	24 May 2019 by Alexander Olenyev <sasha@techmaker.ua>
*
*	Changelog:
*		- v1.8 added support for ILI9340_INV
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

#include "lcd.h"

static int16_t m_width;
static int16_t m_height;
static int16_t m_cursor_x;
static int16_t m_cursor_y;

static uint16_t m_textcolor;
static uint16_t m_textbgcolor;
static uint8_t m_font;
static uint8_t m_rotation;
static uint8_t m_wrap;

static font_t * fonts[] = {
#ifdef USE_FONT8
					&Font8,
#endif
#ifdef USE_FONT12
					&Font12,
#endif
#ifdef USE_FONT16
					&Font16,
#endif
#ifdef USE_FONT20
					&Font20,
#endif
#ifdef USE_FONT24
					&Font24,
#endif
					};
const static uint8_t fontsNum = sizeof(fonts) / sizeof(fonts[0]);

void delay(uint32_t t);
void GPIO_Init(uint32_t mode);
void LCD_Write8(uint8_t data);
uint8_t LCD_Read8(void);
void LCD_Write8Register8(uint8_t a, uint8_t d);
void LCD_Write16Register8(uint8_t a, uint16_t d);
void LCD_Write24Register8(uint8_t a, uint32_t d);
void LCD_Write32Register8(uint8_t a, uint32_t d);
void LCD_Write16Register16(uint16_t a, uint16_t d);
void LCD_Write16RegisterPair8(uint8_t aH, uint8_t aL, uint16_t d);
uint8_t LCD_Read8Register8(uint8_t a, uint8_t dummy);
uint32_t LCD_Color565_to_888(uint16_t color);
uint8_t LCD_Color565_to_R(uint16_t color);
uint8_t LCD_Color565_to_G(uint16_t color);
uint8_t LCD_Color565_to_B(uint16_t color);

// Initialization command tables for different LCD controllers
#if	defined(ILI9325) || defined (ILI9328)
static const uint16_t ILI932x_regValues[] = {
	ILI932X_START_OSC        , 0x0001, // Start oscillator
	TFTLCD_DELAY             , 50,     // 50 millisecond delay
	ILI932X_DRIV_OUT_CTRL    , 0x0100,
	ILI932X_DRIV_WAV_CTRL    , 0x0700,
	ILI932X_ENTRY_MOD        , 0x1030,
	ILI932X_RESIZE_CTRL      , 0x0000,
	ILI932X_DISP_CTRL2       , 0x0202,
	ILI932X_DISP_CTRL3       , 0x0000,
	ILI932X_DISP_CTRL4       , 0x0000,
	ILI932X_RGB_DISP_IF_CTRL1, 0x0,
	ILI932X_FRM_MARKER_POS   , 0x0,
	ILI932X_RGB_DISP_IF_CTRL2, 0x0,
	ILI932X_POW_CTRL1        , 0x0000,
	ILI932X_POW_CTRL2        , 0x0007,
	ILI932X_POW_CTRL3        , 0x0000,
	ILI932X_POW_CTRL4        , 0x0000,
	TFTLCD_DELAY             , 200,
	ILI932X_POW_CTRL1        , 0x1690,
	ILI932X_POW_CTRL2        , 0x0227,
	TFTLCD_DELAY             , 50,
	ILI932X_POW_CTRL3        , 0x001A,
	TFTLCD_DELAY             , 50,
	ILI932X_POW_CTRL4        , 0x1800,
	ILI932X_POW_CTRL7        , 0x002A,
	TFTLCD_DELAY             , 50,
	ILI932X_GAMMA_CTRL1      , 0x0000,
	ILI932X_GAMMA_CTRL2      , 0x0000,
	ILI932X_GAMMA_CTRL3      , 0x0000,
	ILI932X_GAMMA_CTRL4      , 0x0206,
	ILI932X_GAMMA_CTRL5      , 0x0808,
	ILI932X_GAMMA_CTRL6      , 0x0007,
	ILI932X_GAMMA_CTRL7      , 0x0201,
	ILI932X_GAMMA_CTRL8      , 0x0000,
	ILI932X_GAMMA_CTRL9      , 0x0000,
	ILI932X_GAMMA_CTRL10     , 0x0000,
	ILI932X_GRAM_HOR_AD      , 0x0000,
	ILI932X_GRAM_VER_AD      , 0x0000,
	ILI932X_HOR_START_AD     , 0x0000,
	ILI932X_HOR_END_AD       , 0x00EF,
	ILI932X_VER_START_AD     , 0X0000,
	ILI932X_VER_END_AD       , 0x013F,
	ILI932X_GATE_SCAN_CTRL1  , 0xA700, // Driver Output Control (R60h)
	ILI932X_GATE_SCAN_CTRL2  , 0x0003, // Driver Output Control (R61h)
	ILI932X_GATE_SCAN_CTRL3  , 0x0000, // Driver Output Control (R62h)
	ILI932X_PANEL_IF_CTRL1   , 0X0010, // Panel Interface Control 1 (R90h)
	ILI932X_PANEL_IF_CTRL2   , 0X0000,
	ILI932X_PANEL_IF_CTRL3   , 0X0003,
	ILI932X_PANEL_IF_CTRL4   , 0X1100,
	ILI932X_PANEL_IF_CTRL5   , 0X0000,
	ILI932X_PANEL_IF_CTRL6   , 0X0000,
	ILI932X_DISP_CTRL1       , 0x0133, // Main screen turn on
};
#elif defined(UNKNOWN1602)
static const uint8_t ILI9329_regValues[] = {
	ILI9341_SOFTRESET,			0,            //Soft Reset
	TFTLCD_DELAY,				150,// .kbv will power up with ONLY reset, sleep out, display on
	ILI9341_DISPLAYOFF,			0,//Display Off
	ILI9341_PIXELFORMAT,		1, 0x55,//Pixel read=565, write=565.
	//0xF6,						3, 0x01, 0x01, 0x00,  //Interface Control needs EXTC=1 MX_EOR=1, TM=0, RIM=0
	//0xB6,						3, 0x0A, 0x82, 0x27,  //Display Function [0A 82 27]
	//0xB7,						1, 0x06,      //Entry Mode Set [06]
	//0x36,						1, 0x00,//Memory Access [00] pointless but stops an empty array
	ILI9341_SLEEPOUT,			0,//Sleep Out
	TFTLCD_DELAY,				150,
	ILI9341_DISPLAYON,			0,//Display On
	TFTLCD_DELAY,				100,
	ILI9341_MEMORYWRITE,		0,
};
#elif defined(ILI9340)
static const uint8_t ILI9340_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	ILI9341_POWERCONTROLA		, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	ILI9341_POWERCONTROLB		, 3, 0x00, 0xC1, 0x30,
	ILI9341_DRIVERTIMINGCTLA	, 3, 0x85, 0x00, 0x78,
	ILI9341_DRIVERTIMINGCTLB	, 2, 0x00, 0x00,
	ILI9341_POWERONSEQCONTROL	, 4, 0x64, 0x03, 0x12, 0x81,
	ILI9341_PUMPRATIOCONTROL	, 1, 0x20,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x3E, 0x28,
	ILI9341_VCOMCONTROL2		, 1, 0x86,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR, // change rotation here
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x18,
	ILI9341_DISPLAYFUNC			, 3, 0x08, 0x82, 0x27,
	ILI9341_ENABLE3G			, 1, 0x00,
	ILI9341_GAMMASET			, 1, 0x01,
	ILI9341_POSITIVEGAMMACORR	,15, 0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00,
	ILI9341_NEGATIVEGAMMACORR	,15, 0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f,
	ILI9341_INVERTON			, 0,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(ILI9340_INV)
static const uint8_t ILI9340_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	ILI9341_POWERCONTROLA		, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	ILI9341_POWERCONTROLB		, 3, 0x00, 0xC1, 0x30,
	ILI9341_DRIVERTIMINGCTLA	, 3, 0x85, 0x00, 0x78,
	ILI9341_DRIVERTIMINGCTLB	, 2, 0x00, 0x00,
	ILI9341_POWERONSEQCONTROL	, 4, 0x64, 0x03, 0x12, 0x81,
	ILI9341_PUMPRATIOCONTROL	, 1, 0x20,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x3E, 0x28,
	ILI9341_VCOMCONTROL2		, 1, 0x86,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR, // change rotation here
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x18,
	ILI9341_DISPLAYFUNC			, 3, 0x08, 0x82, 0x27,
	ILI9341_ENABLE3G			, 1, 0x00,
	ILI9341_GAMMASET			, 1, 0x01,
	ILI9341_POSITIVEGAMMACORR	,15, 0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00,
	ILI9341_NEGATIVEGAMMACORR	,15, 0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f,
	ILI9341_INVERTOFF			, 0,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(ILI9341)
static const uint8_t ILI9341_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	ILI9341_POWERCONTROLA		, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	ILI9341_POWERCONTROLB		, 3, 0x00, 0xC1, 0x30,
	ILI9341_DRIVERTIMINGCTLA	, 3, 0x85, 0x00, 0x78,
	ILI9341_DRIVERTIMINGCTLB	, 2, 0x00, 0x00,
	ILI9341_POWERONSEQCONTROL	, 4, 0x64, 0x03, 0x12, 0x81,
	ILI9341_PUMPRATIOCONTROL	, 1, 0x20,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x3E, 0x28,
	ILI9341_VCOMCONTROL2		, 1, 0x86,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR, // change rotation here
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x18,
	ILI9341_DISPLAYFUNC			, 3, 0x08, 0x82, 0x27,
	ILI9341_ENABLE3G			, 1, 0x00,
	ILI9341_GAMMASET			, 1, 0x01,
	ILI9341_POSITIVEGAMMACORR	,15, 0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00,
	ILI9341_NEGATIVEGAMMACORR	,15, 0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(ILI9341_00)
static const uint8_t ILI9341_00_regValues[] = {
	ILI9341_SOFTRESET			, 0,
	TFTLCD_DELAY				, 50,
	ILI9341_DISPLAYOFF			, 0,
	ILI9341_POWERCONTROL1		, 1, 0x23,
	ILI9341_POWERCONTROL2		, 1, 0x10,
	ILI9341_VCOMCONTROL1		, 2, 0x2B, 0x2B,
	ILI9341_VCOMCONTROL2		, 1, 0xC0,
	ILI9341_MEMCONTROL			, 1, ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR, // change rotation here
	ILI9341_PIXELFORMAT			, 1, 0x55,
	ILI9341_FRAMECONTROL		, 2, 0x00, 0x1B,
	ILI9341_ENTRYMODE			, 1, 0x07,
	ILI9341_INVERTON			, 0,
	ILI9341_SLEEPOUT			, 0,
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,
	TFTLCD_DELAY				, 100,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(R61505)
static const uint16_t ILI9320_regValues[] = {
	0x00e5, 0x8000,
	0x0000, 0x0001,
	0x0001, 0x0100,
	0x0002, 0x0700,
	0x0003, 0x1030,
	0x0004, 0x0000,
	0x0008, 0x0202,
	0x0009, 0x0000,
	0x000A, 0x0000,
	0x000C, 0x0000,
	0x000D, 0x0000,
	0x000F, 0x0000,
	//-----Power On sequence-----------------------
	0x0010, 0x0000,
	0x0011, 0x0007,
	0x0012, 0x0000,
	0x0013, 0x0000,
	TFTLCD_DELAY, 50,
	0x0010, 0x17B0,  //SAP=1, BT=7, APE=1, AP=3
	0x0011, 0x0007,  //DC1=0, DC0=0, VC=7
	TFTLCD_DELAY, 10,
	0x0012, 0x013A,  //VCMR=1, PON=3, VRH=10
	TFTLCD_DELAY, 10,
	0x0013, 0x1A00,  //VDV=26
	0x0029, 0x000c,  //VCM=12
	TFTLCD_DELAY, 10,
	//-----Gamma control-----------------------
	0x0030, 0x0000,
	0x0031, 0x0505,
	0x0032, 0x0004,
	0x0035, 0x0006,
	0x0036, 0x0707,
	0x0037, 0x0105,
	0x0038, 0x0002,
	0x0039, 0x0707,
	0x003C, 0x0704,
	0x003D, 0x0807,
	//-----Set RAM area-----------------------
	0x0060, 0x2700,     //GS=0!
	0x0061, 0x0001,
	0x006A, 0x0000,
	0x0021, 0x0000,
	0x0020, 0x0000,
	//-----Partial Display Control------------
	0x0080, 0x0000,
	0x0081, 0x0000,
	0x0082, 0x0000,
	0x0083, 0x0000,
	0x0084, 0x0000,
	0x0085, 0x0000,
	//-----Panel Control----------------------
	0x0090, 0x0010,
	0x0092, 0x0000,
	0x0093, 0x0003,
	0x0095, 0x0110,
	0x0097, 0x0000,
	0x0098, 0x0000,
	//-----Display on-----------------------
	0x0007, 0x0173,
	TFTLCD_DELAY, 50,
};
#elif defined(R61505V)
static const uint16_t R61505V_regValues[] = {
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0000,
	0x0000, 0x0001,
	0x00A4, 0x0001,     //CALB=1
	TFTLCD_DELAY, 10,
	0x0060, 0x2700,     //NL
	0x0008, 0x0808,     //FP & BP
	0x0030, 0x0214,     //Gamma settings
	0x0031, 0x3715,
	0x0032, 0x0604,
	0x0033, 0x0E16,
	0x0034, 0x2211,
	0x0035, 0x1500,
	0x0036, 0x8507,
	0x0037, 0x1407,
	0x0038, 0x1403,
	0x0039, 0x0020,
	0x0090, 0x0015,     //DIVI & RTNI
	0x0010, 0x0410,     //BT,AP
	0x0011, 0x0237,     //VC,DC0,DC1
	0x0029, 0x0046,     //VCM1
	0x002A, 0x0046,     //VCMSEL,VCM2
	// Sleep mode IN sequence
	0x0007, 0x0000,
	//0x0012, 0x0000,   //PSON=0,PON=0
	// Sleep mode EXIT sequence
	0x0012, 0x0189,     //VRH=9,VCMR=1,PSON=0,PON=0
	0x0013, 0x1100,     //VDV
	TFTLCD_DELAY, 150,
	0x0012, 0x01B9,     //VRH=9,VCMR=1,PSON=1,PON=1 [018F]
	0x0001, 0x0100,     //SS=1 Other mode settings
	0x0002, 0x0200,     //BC0=1--Line inversion
	0x0003, 0x1030,
	0x0009, 0x0001,     //ISC=1 [0000]
	0x000A, 0x0000,     // [0000]
	//0x000C, 0x0001,   //RIM=1 [0000]
	0x000D, 0x0000,     // [0000]
	0x000E, 0x0030,     //VEM=3 VCOM equalize [0000]
	0x0061, 0x0001,
	0x006A, 0x0000,
	0x0080, 0x0000,
	0x0081, 0x0000,
	0x0082, 0x005F,
	0x0092, 0x0100,
	0x0093, 0x0701,
	TFTLCD_DELAY, 80,
	0x0007, 0x0100,     //BASEE=1--Display On
};
#elif defined(R61520)
static const uint8_t R61520_regValues[] = { //AUTO_READINC | MIPI_DCS_REV1 | MV_AXIS | READ_24BITS
	ILI9341_SOFTRESET			, 0,            //Soft Reset
	TFTLCD_DELAY				, 150,  // .kbv will power up with ONLY reset, sleep out, display on
	ILI9341_DISPLAYOFF			, 0,            //Display Off
	ILI9341_PIXELFORMAT			, 1, 0x55,      //Pixel read=565, write=565.
    0xB0						, 1, 0x00,      //Command Access Protect
    0xC0						, 1, 0x08 | 0x02 | 0x01,// DM = 1, BGR = 1, MH = 1 // DM - SPLITFIX HERE!!!!
	ILI9341_POSITIVEGAMMACORR	,15, 0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00,
	ILI9341_NEGATIVEGAMMACORR	,15, 0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f,
	ILI9341_SLEEPOUT			, 0,            //Sleep Out
	TFTLCD_DELAY				, 150,
	ILI9341_DISPLAYON			, 0,            //Display On
	TFTLCD_DELAY				, 100,
	ILI9341_MEMORYWRITE			, 0,
};
#elif defined(S6D0154)
static const uint16_t S6D0154_regValues[] = {
	0x0011, 0x001A,
	0x0012, 0x3121,
	0x0013, 0x006C,
	0x0014, 0x4249,
	0x0010, 0x0800,
	TFTLCD_DELAY, 10,
	0x0011, 0x011A,
	TFTLCD_DELAY, 10,
	0x0011, 0x031A,
	TFTLCD_DELAY, 10,
	0x0011, 0x071A,
	TFTLCD_DELAY, 10,
	0x0011, 0x0F1A,
	TFTLCD_DELAY, 10,
	0x0011, 0x0F3A,
	TFTLCD_DELAY, 30,
	0x0001, 0x0128,
	0x0002, 0x0100,
	0x0003, 0x1030,
	0x0007, 0x1012,
	0x0008, 0x0303,
	0x000B, 0x1100,
	0x000C, 0x0000,
	0x000F, 0x1801,
	0x0015, 0x0020,
	/*
	0x0050,0x0101,
	0x0051,0x0603,
	0x0052,0x0408,
	0x0053,0x0000,
	0x0054,0x0605,
	0x0055,0x0406,
	0x0056,0x0303,
	0x0057,0x0303,
	0x0058,0x0010,
	0x0059,0x1000,
	*/
	0x0007, 0x0012,
	TFTLCD_DELAY, 40,
	0x0007, 0x0013,     /* GRAM Address Set */
	0x0007, 0x0017,     /* Display Control DISPLAY ON */
};
#elif defined(HX8347D)
static const uint8_t HX8347D_regValues[] = {
	0x2E, 0x89,
	0x29, 0x8F,
	0x2B, 0x02,
	0xE2, 0x00,
	0xE4, 0x01,
	0xE5, 0x10,
	0xE6, 0x01,
	0xE7, 0x10,
	0xE8, 0x70,
	0xF2, 0x00,
	0xEA, 0x00,
	0xEB, 0x20,
	0xEC, 0x3C,
	0xED, 0xC8,
	0xE9, 0x38,
	0xF1, 0x01,
	// skip gamma, do later
	0x1B, 0x1A,
	0x1A, 0x02,
	0x24, 0x61,
	0x25, 0x5C,
	0x18, 0x36,
	0x19, 0x01,
	0x1F, 0x88,
	TFTLCD_DELAY, 5,// delay 5 ms
	0x1F, 0x80,
	TFTLCD_DELAY, 5,
	0x1F, 0x90,
	TFTLCD_DELAY, 5,
	0x1F, 0xD4,
	TFTLCD_DELAY, 5,
	0x17, 0x05,
	0x36, 0x01,	// MADCTL: 0x01 BGR=1, 0x02 INVERTCOLOR=1, 0x04 MY=1, 0x08 MX=1
	0x28, 0x38,
	TFTLCD_DELAY, 40,
	0x28, 0x3C,
	0x02, 0x00,
	0x03, 0x00,
	0x04, 0x00,
	0x05, 0xEF,
	0x06, 0x00,
	0x07, 0x00,
	0x08, 0x01,
	0x09, 0x3F
};
#elif defined(HX8347G)
static const uint8_t HX8347G_regValues[] = {
	0x2E, 0x89,
	0x29, 0x8F,
	0x2B, 0x02,
	0xE2, 0x00,
	0xE4, 0x01,
	0xE5, 0x10,
	0xE6, 0x01,
	0xE7, 0x10,
	0xE8, 0x70,
	0xF2, 0x00,
	0xEA, 0x00,
	0xEB, 0x20,
	0xEC, 0x3C,
	0xED, 0xC8,
	0xE9, 0x38,
	0xF1, 0x01,
	// skip gamma, do later
	0x1B, 0x1A,
	0x1A, 0x02,
	0x24, 0x61,
	0x25, 0x5C,
	0x18, 0x36,
	0x19, 0x01,
	0x1F, 0x88,
	TFTLCD_DELAY, 5,// delay 5 ms
	0x1F, 0x80,
	TFTLCD_DELAY, 5,
	0x1F, 0x90,
	TFTLCD_DELAY, 5,
	0x1F, 0xD4,
	TFTLCD_DELAY, 5,
	0x17, 0x05,
	0x36, 0x09,
	0x28, 0x38,
	TFTLCD_DELAY, 40,
	0x28, 0x3C,
	0x02, 0x00,
	0x03, 0x00,
	0x04, 0x00,
	0x05, 0xEF,
	0x06, 0x00,
	0x07, 0x00,
	0x08, 0x01,
	0x09, 0x3F
};
#elif defined(HX8357D)
static const uint8_t HX8357D_regValues[] = {
	HX8357_SWRESET, 0,
	HX8357D_SETC, 3, 0xFF, 0x83, 0x57,
	TFTLCD_DELAY, 250,
	HX8357_SETRGB, 4, 0x00, 0x00, 0x06, 0x06,
	HX8357D_SETCOM, 1, 0x25,  // -1.52V
	HX8357_SETOSC, 1, 0x68,  // Normal mode 70Hz, Idle mode 55 Hz
	HX8357_SETPANEL, 1, 0x05,  // BGR, Gate direction swapped
	HX8357_SETPWR1, 6, 0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA,
	HX8357D_SETSTBA, 6, 0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08,
	// MEME GAMMA HERE
	HX8357D_SETCYC, 7, 0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78,
	HX8357_COLMOD, 1, 0x55,
	HX8357_MADCTL, 1, 0xC0,
	HX8357_TEON, 1, 0x00,
	HX8357_TEARLINE, 2, 0x00, 0x02,
	HX8357_SLPOUT, 0,
	TFTLCD_DELAY, 150,
	HX8357_DISPON, 0,
	TFTLCD_DELAY, 50,
};
#elif defined(ST7781)
static const uint16_t ST7781_regValues[] = {
	0x0001, 0x0100,
	0x0002, 0x0700,
	0x0003, 0x1030,
	0x0008, 0x0302,
	0x0009, 0x0000,
	0x000A, 0x0008,
	//*******POWER CONTROL REGISTER INITIAL*******//
	0x0010, 0x0790,
	0x0011, 0x0005,
	0x0012, 0x0000,
	0x0013, 0x0000,
	//delayms(50,
	//********POWER SUPPPLY STARTUP 1 SETTING*******//
	0x0010, 0x12B0,
	// delayms(50,
	0x0011, 0x0007,
	//delayms(50,
	//********POWER SUPPLY STARTUP 2 SETTING******//
	0x0012, 0x008C,
	0x0013, 0x1700,
	0x0029, 0x0022,
	// delayms(50,
	//******GAMMA CLUSTER SETTING******//
	0x0030, 0x0000,
	0x0031, 0x0505,
	0x0032, 0x0205,
	0x0035, 0x0206,
	0x0036, 0x0408,
	0x0037, 0x0000,
	0x0038, 0x0504,
	0x0039, 0x0206,
	0x003C, 0x0206,
	0x003D, 0x0408,
	// -----------DISPLAY WINDOWS 240*320-------------//
	0x0050, 0x0000,
	0x0051, 0x00EF,
	0x0052, 0x0000,
	0x0053, 0x013F,
	//-----FRAME RATE SETTING-------//
	0x0060, 0xA700,
	0x0061, 0x0001,
	0x0090, 0x0033,//RTNI setting
	//-------DISPLAY ON------//
	0x0007, 0x0133,
	0x0001, 0x0100,
	0x0002, 0x0700,
	0x0003, 0x1030,
	0x0008, 0x0302,
	0x0009, 0x0000,
	0x000A, 0x0008,
	//*******POWER CONTROL REGISTER INITIAL*******//
	0x0010, 0x0790,
	0x0011, 0x0005,
	0x0012, 0x0000,
	0x0013, 0x0000,
	TFTLCD_DELAY, 5,
	//********POWER SUPPPLY STARTUP 1 SETTING*******//
	0x0010, 0x12B0,
	TFTLCD_DELAY, 5,
	0x0011, 0x0007,
	TFTLCD_DELAY, 5,
	//********POWER SUPPLY STARTUP 2 SETTING******//
	0x0012, 0x008C,
	0x0013, 0x1700,
	0x0029, 0x0022,
	TFTLCD_DELAY, 5,
	//******GAMMA CLUSTER SETTING******//
	0x0030, 0x0000,
	0x0031, 0x0505,
	0x0032, 0x0205,
	0x0035, 0x0206,
	0x0036, 0x0408,
	0x0037, 0x0000,
	0x0038, 0x0504,
	0x0039, 0x0206,
	0x003C, 0x0206,
	0x003D, 0x0408,
	// -----------DISPLAY WINDOWS 240*320-------------//
	0x0050, 0x0000,
	0x0051, 0x00EF,
	0x0052, 0x0000,
	0x0053, 0x013F,
	//-----FRAME RATE SETTING-------//
	0x0060, 0xA700,
	0x0061, 0x0001,
	0x0090, 0x0033, //RTNI setting
	//-------DISPLAY ON------//
	0x0007, 0x0133,
	TFTLCD_DELAY, 50,
};
#elif defined (SSD1297)
// came from MikroElektronika library http://www.hmsprojects.com/tft_lcd.html
static const uint16_t SSD1297_regValues[] = {
        0x0000, 0x0001,
        0x0003, 0xA8A4,
        0x000C, 0x0000,
        0x000D, 0x000A,     // VRH=10
        0x000E, 0x2B00,
        0x001E, 0x00B7,
        0x0001, 0x2B3F,     // setRotation() alters
        0x0002, 0x0600,     // B_C=1, EOR=1
        0x0010, 0x0000,
        0x0011, 0x4C30,     // setRotation() alters
        0x0005, 0x0000,
        0x0006, 0x0000,
        0x0016, 0xEF1C,
        0x0017, 0x0003,
        0x0007, 0x0233,
        0x000B, 0x0000,
        0x000F, 0x0000,
        0x0030, 0x0707,
        0x0031, 0x0204,
        0x0032, 0x0204,
        0x0033, 0x0502,
        0x0034, 0x0507,
        0x0035, 0x0204,
        0x0036, 0x0204,
        0x0037, 0x0502,
        0x003A, 0x0302,
        0x003B, 0x0302,
        0x0023, 0x0000,
        0x0024, 0x0000,
        0x0025, 0x8000
};
#endif

/**
 * \brief Makes a delays for t processor cycles
 *
 * \param t		Number of "nop" cycles
 *
 * \return void
 */		
inline void delay(uint32_t t) {
	for (; t; t--) __asm("nop");
}

/**
 * \brief GPIO Initialization
 *
 * \param
 *
 * \return void
 */
void GPIO_Init(uint32_t mode) {

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

	if (!(mode == GPIO_MODE_OUTPUT_PP || mode == GPIO_MODE_INPUT)) return;

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/*Configure GPIO data pins : PA8 PA9 PA10 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO data pins : PB3 PB4 PB5 PB10 */
	GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO data pins : PC7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO control pins : PA0 PA1 PA4 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO control pins : PB0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO control pins : PC1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
 * \brief Writes 8-Bit data
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
inline void LCD_Write8(uint8_t data) {
	// ------ PORT -----     --- Data ----
	// GPIOA, GPIO_PIN_9  -> BIT 0 -> 0x01
	// GPIOC, GPIO_PIN_7  -> BIT 1 -> 0x02
	// GPIOA, GPIO_PIN_10 -> BIT 2 -> 0x04
	// GPIOB, GPIO_PIN_3  -> BIT 3 -> 0x08
	// GPIOB, GPIO_PIN_5  -> BIT 4 -> 0x10
	// GPIOB, GPIO_PIN_4  -> BIT 5 -> 0x20
	// GPIOB, GPIO_PIN_10 -> BIT 6 -> 0x40
	// GPIOA, GPIO_PIN_8  -> BIT 7 -> 0x80

	GPIOA->ODR = (GPIOA->ODR & 0xF8FF) | ((data & 0x01) << 9) | ((data & 0x04) << 8) | ((data & 0x80) << 1);
	GPIOB->ODR = (GPIOB->ODR & 0xFBC7) | (data & 0x08) | ((data & 0x10) << 1) | ((data & 0x20) >> 1) | ((data & 0x40) << 4);
	GPIOC->ODR = (GPIOC->ODR & 0xFF7F) | ((data & 0x02) << 6);

	LCD_WR_STROBE();
}

/**
 * \brief Reads 8-Bit data
 *
 * \param data	8-Bit Data
 *
 * \return void
 */
inline uint8_t LCD_Read8(void) {
	// - Data - ----------- PORT -----------
	// BIT 0 -> GPIOA, GPIO_PIN_9  -> 0x0200
	// BIT 1 -> GPIOC, GPIO_PIN_7  -> 0x0080
	// BIT 2 -> GPIOA, GPIO_PIN_10 -> 0x0400
	// BIT 3 -> GPIOB, GPIO_PIN_3  -> 0x0008
	// BIT 4 -> GPIOB, GPIO_PIN_5  -> 0x0020
	// BIT 5 -> GPIOB, GPIO_PIN_4  -> 0x0010
	// BIT 6 -> GPIOB, GPIO_PIN_10 -> 0x0400
	// BIT 7 -> GPIOA, GPIO_PIN_8  -> 0x0100
	uint8_t data;
	LCD_RD_STROBE();
	data = ((GPIOA->IDR & 0x0200) >> 9) | ((GPIOA->IDR & 0x0400) >> 8) | ((GPIOA->IDR & 0x0100) >> 1) | (GPIOB->IDR & 0x0008)
			| ((GPIOB->IDR & 0x0020) >> 1) | ((GPIOB->IDR & 0x0010) << 1) | ((GPIOB->IDR & 0x0400) >> 4)
			| ((GPIOC->IDR & 0x0080) >> 6);
	return data;
}

/**
 * \brief Writes 8-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write8Register8(uint8_t a, uint8_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16Register8(uint8_t a, uint16_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 24-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write24Register8(uint8_t a, uint32_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 32-Bit data to register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write32Register8(uint8_t a, uint32_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 24);
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to register (16-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16Register16(uint16_t a, uint16_t d) {
	LCD_CD_COMMAND();
	LCD_Write8(a >> 8);
	LCD_Write8(a);
	LCD_CD_DATA();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}

/**
 * \brief Writes 16-Bit data to two registers (8-bit address)
 *
 * \param aH	Register for hi chunk of data
 * \param aL	Register for lo chunk of data
 * \param d		Data
 *
 * \return void
 */
inline void LCD_Write16RegisterPair8(uint8_t aH, uint8_t aL, uint16_t d) {
	LCD_Write8Register8(aH, d >> 8);
	LCD_Write8Register8(aL, d);
}

/**
 * \brief Reads 8-Bit data from register (8-bit address)
 *
 * \param a		Register
 * \param d		Data
 *
 * \return void
 */
inline uint8_t LCD_Read8Register8(uint8_t a, uint8_t dummy) {
	uint8_t data;
	LCD_CD_COMMAND();
	LCD_Write8(a);
	GPIO_Init(GPIO_MODE_INPUT);
	LCD_CD_DATA();
	if (dummy) LCD_RD_STROBE();
	data = LCD_Read8();
	GPIO_Init(GPIO_MODE_OUTPUT_PP);
	return data;
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
	m_rotation = 0;
	m_cursor_y = m_cursor_x = 0;
	m_font = 0;
	m_textcolor = m_textbgcolor = 0xFFFF;
	m_wrap = 1;

	GPIO_Init(GPIO_MODE_OUTPUT_PP);

	LCD_Reset();
	HAL_Delay(50);

	LCD_CS_ACTIVE();

#if	defined(ILI9325) || defined (ILI9328)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(ILI932x_regValues) / sizeof(ILI932x_regValues[0])) {
		a = ILI932x_regValues[i++];
		d = ILI932x_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register16(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(UNKNOWN1602)	
	uint8_t i = 0;
	while(i < sizeof(ILI9329_regValues)/sizeof(ILI9329_regValues[0])) {
		uint8_t r = ILI9329_regValues[i++];
		uint8_t len = ILI9329_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9329_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, TFTHEIGHT - 1);
#elif defined(ILI9340)
	uint8_t i = 0;
	while(i < sizeof(ILI9340_regValues)/sizeof(ILI9340_regValues[0])) {
		uint8_t r = ILI9340_regValues[i++];
		uint8_t len = ILI9340_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9340_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(ILI9341)
	uint8_t i = 0;
	while(i < sizeof(ILI9341_regValues)/sizeof(ILI9341_regValues[0])) {
		uint8_t r = ILI9341_regValues[i++];
		uint8_t len = ILI9341_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9341_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(ILI9341_00)
	uint8_t i = 0;
	while(i < sizeof(ILI9341_00_regValues)/sizeof(ILI9341_00_regValues[0])) {
		uint8_t r = ILI9341_00_regValues[i++];
		uint8_t len = ILI9341_00_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = ILI9341_00_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(R61505)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(ILI9320_regValues) / sizeof(ILI9320_regValues[0])) {
		a = ILI9320_regValues[i++];
		d = ILI9320_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register16(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(R61505V)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(R61505V_regValues) / sizeof(R61505V_regValues[0])) {
		a = R61505V_regValues[i++];
		d = R61505V_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register16(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(R61520)
	uint8_t i = 0;
	while(i < sizeof(R61520_regValues)/sizeof(R61520_regValues[0])) {
		uint8_t r = R61520_regValues[i++];
		uint8_t len = R61520_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = R61520_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(S6D0154)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(S6D0154_regValues) / sizeof(S6D0154_regValues[0])) {
		a = S6D0154_regValues[i++];
		d = S6D0154_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register16(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(HX8347D)
	uint8_t i = 0;
	uint16_t a, d;
	while(i < sizeof(HX8347D_regValues)) {
		a = HX8347D_regValues[i++];
		d = HX8347D_regValues[i++];
		if(a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write8Register8(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetLR(); // Lower-right corner of address window
#elif defined(HX8347G)
	uint8_t i = 0;
	uint16_t a, d;
	while(i < sizeof(HX8347G_regValues)) {
		a = HX8347G_regValues[i++];
		d = HX8347G_regValues[i++];
		if(a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write8Register8(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetLR(); // Lower-right corner of address window
#elif defined(HX8357D)
	uint8_t i = 0;
	while(i < sizeof(HX8357D_regValues) / sizeof(ST7781_regValues[0])) {
		uint8_t r = HX8357D_regValues[i++];
		uint8_t len = HX8357D_regValues[i++];
		if(r == TFTLCD_DELAY) {
			HAL_Delay(len);
		} else {
			LCD_CD_COMMAND();
			LCD_Write8(r);
			LCD_CD_DATA();
			for (uint8_t d = 0; d < len; d++) {
				uint8_t x = HX8357D_regValues[i++];
				LCD_Write8(x);
			}
		}
	}
#elif defined(ST7781)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(ST7781_regValues) / sizeof(ST7781_regValues[0])) {
		a = ST7781_regValues[i++];
		d = ST7781_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register16(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(SSD1297)
	uint8_t i = 0;
	uint16_t a, d;
	while (i < sizeof(SSD1297_regValues) / sizeof(SSD1297_regValues[0])) {
		a = SSD1297_regValues[i++];
		d = SSD1297_regValues[i++];
		if (a == TFTLCD_DELAY) {
			HAL_Delay(d);
		} else {
			LCD_Write16Register8(a, d);
		}
	}
	LCD_SetRotation(m_rotation);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#endif

	LCD_CS_IDLE();
	LCD_FillScreen(BLACK);
	LCD_SetTextSize(0);
	LCD_SetTextColor(WHITE, BLACK);
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
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) return;

	LCD_CS_ACTIVE();

#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
	int16_t t;
	switch (m_rotation) {
	case 1:
		t = x;
		x = TFTWIDTH - 1 - y;
		y = t;
		break;
	case 2:
		x = TFTWIDTH - 1 - x;
		y = TFTHEIGHT - 1 - y;
		break;
	case 3:
		t = x;
		x = y;
		y = TFTHEIGHT - 1 - t;
		break;
	}
	LCD_Write16Register16(ILI932X_GRAM_HOR_AD, x);
	LCD_Write16Register16(ILI932X_GRAM_VER_AD, y);
	LCD_Write16Register16(ILI932X_GRAM_WR, color);
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520) || defined(UNKNOWN1602)	
	LCD_SetAddrWindow(x, y, m_width - 1, m_height - 1);
	LCD_CS_ACTIVE();
	LCD_Write16Register8(ILI9341_MEMORYWRITE, color);
#elif defined(HX8347D) || defined(HX8347G)
    uint8_t t;
    switch(m_rotation) {
		case 1: t = 0x60; break;
		case 2: t = 0xc0; break;
		case 3: t = 0xa0; break;
		default: t = 0; break;
    }
    LCD_Write8Register8(HX8347G_MEMACCESS, t);
    // Only upper-left is set -- bottom-right is full screen default
    LCD_Write16RegisterPair8(HX8347G_COLADDRSTART_HI, HX8347G_COLADDRSTART_LO, x);
    LCD_Write16RegisterPair8(HX8347G_ROWADDRSTART_HI, HX8347G_ROWADDRSTART_LO, y);
    LCD_Write16Register8(HX8347G_SRAM_WR, color);
#elif defined(HX8357D)
	LCD_SetAddrWindow(x, y, m_width - 1, m_height - 1);
	LCD_CS_ACTIVE();
	LCD_Write16Register8(HX8357_RAMWR, color);
#elif defined(SSD1297)
	if(m_rotation & 1) {
		LCD_Write16Register8(SSD1297_SETXCOUNTER, y); //GRAM Address Set
		LCD_Write16Register8(SSD1297_SETYCOUNTER, x);
	} else {
		LCD_Write16Register8(SSD1297_SETXCOUNTER, x); //GRAM Address Set
		LCD_Write16Register8(SSD1297_SETYCOUNTER, y);
	}
	LCD_Write24Register8(SSD1297_RAMDATA_WRITE, LCD_Color565_to_888(color));
#endif

	LCD_CS_IDLE();
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
#if defined(SSD1297)
	uint8_t red = LCD_Color565_to_R(color);
	uint8_t green = LCD_Color565_to_G(color);
	uint8_t blue = LCD_Color565_to_B(color);
#else
	uint8_t hi = color >> 8, lo = color;
#endif

	LCD_CS_ACTIVE();

#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
	LCD_Write16Register16(ILI932X_GRAM_WR, color);
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520) || defined(UNKNOWN1602)
	LCD_Write16Register8(ILI9341_MEMORYWRITE, color);
#elif defined(HX8347D) || defined(HX8347G)
	LCD_Write16Register8(HX8347G_SRAM_WR, color);
#elif defined(HX8357D)
	LCD_Write16Register8(HX8357_RAMWR, color);
#elif defined(SSD1297)
	LCD_Write24Register8(SSD1297_RAMDATA_WRITE, LCD_Color565_to_888(color));
#endif
	len--;
#if defined(SSD1297)
	if ((red == green) && (green == blue) && (red == blue)) {
#else
	if (hi == lo) {
#endif
		// High and low bytes are identical.  Leave prior data
		// on the port(s) and just toggle the write strobe.
		while (len--) {
#if defined(SSD1297)
			LCD_WR_STROBE();
#endif
			LCD_WR_STROBE();
			LCD_WR_STROBE();
		}
	} else {
		while (len--) {
#if defined(SSD1297)
			LCD_Write8(red);
			LCD_Write8(green);
			LCD_Write8(blue);
#else
			LCD_Write8(hi);
			LCD_Write8(lo);
#endif
		}
	}

	LCD_CS_IDLE();
}

/**
 * \brief Fills the screen with the specified color
 *
 * \param color	Color
 *
 * \return void
 */
void LCD_FillScreen(uint16_t color) {
#if defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(SSD1297)
	/* 
		For the 932X, a full-screen address window is already the default
		state, just need to set the address pointer to the top-left corner.
		Although we could fill in any direction, the code uses the current
		screen rotation because some users find it disconcerting when a
		fill does not occur top-to-bottom. 
	*/
	uint16_t x, y;
	switch (m_rotation) {
	default:
		x = 0;
		y = 0;
		break;
	case 1:
		x = TFTWIDTH - 1;
		y = 0;
		break;
	case 2:
		x = TFTWIDTH - 1;
		y = TFTHEIGHT - 1;
		break;
	case 3:
		x = 0;
		y = TFTHEIGHT - 1;
		break;
	}
	LCD_CS_ACTIVE();
#if defined(SSD1297)
	LCD_Write16Register8(SSD1297_SETXCOUNTER, x); //GRAM Address Set
	LCD_Write16Register8(SSD1297_SETYCOUNTER, y);
#else
	LCD_Write16Register16(ILI932X_GRAM_HOR_AD, x);
	LCD_Write16Register16(ILI932X_GRAM_VER_AD, y);
#endif
	LCD_CS_IDLE();
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00)\
	|| defined(R61520) || defined(S6D0154) \
	|| defined(UNKNOWN1602) || defined(HX8347D) || defined(HX8347G) || defined(HX8357D)
	/*
		For these, there is no settable address pointer, instead the
		address window must be set for each drawing operation.  However,
		this display takes rotation into account for the parameters, no
		need to do extra rotation math here.
	*/
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#endif
	LCD_Flood(color, (long) TFTWIDTH * (long) TFTHEIGHT);
}

/**
 * \brief Resets the Display
 *
 * \param
 *
 * \return void
 */
void LCD_Reset(void) {
	LCD_CS_IDLE();
	LCD_CD_DATA();
	LCD_WR_IDLE();
	LCD_RD_IDLE();

	LCD_RST_ACTIVE();
	HAL_Delay(2);
	LCD_RST_IDLE();
	HAL_Delay(120);
	// Data transfer sync
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
	LCD_Write8(0x00);
	for (uint8_t i = 0; i < 3; i++) LCD_WR_STROBE(); // Three extra 0x00s
	LCD_CS_IDLE();
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
void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	LCD_CS_ACTIVE();
#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
	/* 
		Values passed are in current (possibly rotated) coordinate
		system.  932X requires hardware-native coords regardless of
		MADCTL, so rotate inputs as needed.  The address counter is
		set to the top-left corner -- although fill operations can be
		done in any direction, the current screen rotation is applied
		because some users find it disconcerting when a fill does not
		occur top-to-bottom.
	*/
	int x, y, t;
	switch (m_rotation) {
	default:
		x = x1;
		y = y1;
		break;
	case 1:
		t = y1;
		y1 = x1;
		x1 = TFTWIDTH - 1 - y2;
		y2 = x2;
		x2 = TFTWIDTH - 1 - t;
		x = x2;
		y = y1;
		break;
	case 2:
		t = x1;
		x1 = TFTWIDTH - 1 - x2;
		x2 = TFTWIDTH - 1 - t;
		t = y1;
		y1 = TFTHEIGHT - 1 - y2;
		y2 = TFTHEIGHT - 1 - t;
		x = x2;
		y = y2;
		break;
	case 3:
		t = x1;
		x1 = y1;
		y1 = TFTHEIGHT - 1 - x2;
		x2 = y2;
		y2 = TFTHEIGHT - 1 - t;
		x = x1;
		y = y2;
		break;
	}
#if defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V)
	LCD_Write16Register16(ILI932X_HOR_START_AD, x1); // Set address window
	LCD_Write16Register16(ILI932X_HOR_END_AD, x2);
	LCD_Write16Register16(ILI932X_VER_START_AD, y1);
	LCD_Write16Register16(ILI932X_VER_END_AD, y2);
	LCD_Write16Register16(ILI932X_GRAM_HOR_AD, x); // Set address counter to top left
	LCD_Write16Register16(ILI932X_GRAM_VER_AD, y);
#elif defined(S6D0154)
	LCD_Write16Register16(0x37, x1); //HorizontalStartAddress
	LCD_Write16Register16(0x36, x2); //HorizontalEndAddress
	LCD_Write16Register16(0x39, y1); //VerticalStartAddress
	LCD_Write16Register16(0x38, y2); //VertocalEndAddress
	LCD_Write16Register16(0x20, x); //GRAM Address Set
	LCD_Write16Register16(0x21, y);
	LCD_Write8Register8(0x22, 0);
#endif
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520) || defined(UNKNOWN1602)
	LCD_Write32Register8(ILI9341_COLADDRSET, (x1 << 16) | x2);
	LCD_Write32Register8(ILI9341_PAGEADDRSET, (y1 << 16) | y2);
#elif defined(HX8347D) || defined(HX8347G)
    LCD_Write16RegisterPair8(HX8347G_COLADDRSTART_HI, HX8347G_COLADDRSTART_LO, x1);
    LCD_Write16RegisterPair8(HX8347G_COLADDREND_HI, HX8347G_COLADDREND_LO, x2);
    LCD_Write16RegisterPair8(HX8347G_ROWADDRSTART_HI, HX8347G_ROWADDRSTART_LO, y1);
    LCD_Write16RegisterPair8(HX8347G_ROWADDREND_HI, HX8347G_ROWADDREND_LO, y2);
#elif defined(HX8357D)
	LCD_Write32Register8(HX8357_CASET, x1 << 16 | x2);
	LCD_Write32Register8(HX8357_PASET, y1 << 16 | y2);
#elif defined(SSD1297)
	if(m_rotation & 1) {
		LCD_Write16Register8(SSD1297_SETXCOUNTER, y1); //GRAM Address Set
		LCD_Write16Register8(SSD1297_SETYCOUNTER, x1);
		LCD_Write16Register8(SSD1297_SETXADDR, y2 << 8 | y1); //HorizontalStartAddress and HorizontalEndAddress
		LCD_Write16Register8(SSD1297_SETYADDR_START, x1); //VerticalStartAddress
		LCD_Write16Register8(SSD1297_SETYADDR_END, x2); //VerticalEndAddress
	} else {
		LCD_Write16Register8(SSD1297_SETXCOUNTER, x1); //GRAM Address Set
		LCD_Write16Register8(SSD1297_SETYCOUNTER, y1);
		LCD_Write16Register8(SSD1297_SETXADDR, x2 << 8 | x1); //HorizontalStartAddress and HorizontalEndAddress
		LCD_Write16Register8(SSD1297_SETYADDR_START, y1); //VerticalStartAddress
		LCD_Write16Register8(SSD1297_SETYADDR_END, y2); //VerticalEndAddress
	}
#endif
	LCD_CS_IDLE();
}

#if defined(HX8347D) || defined(HX8347G)
/*
	Unlike the 932X drivers that set the address window to the full screen
	by default (using the address counter for drawPixel operations), the
	7575 needs the address window set on all graphics operations.  In order
	to save a few register writes on each pixel drawn, the lower-right
	corner of the address window is reset after most fill operations, so
	that drawPixel only needs to change the upper left each time.
*/
void LCD_SetLR(void) {
	LCD_CS_ACTIVE();
	LCD_Write16RegisterPair8(HX8347G_COLADDREND_HI, HX8347G_COLADDREND_LO, m_width - 1);
	LCD_Write16RegisterPair8(HX8347G_ROWADDREND_HI, HX8347G_ROWADDREND_LO, m_height - 1);
	LCD_CS_IDLE();
}
#endif

/**
 * \brief Set display rotation
 *
 * \param x	rotation
 *
 * \return void
 */
void LCD_SetRotation(uint8_t x) {
	m_rotation = (x & 3);
	switch (m_rotation) {
	default:
	case 0:
	case 2:
		m_width = TFTWIDTH;
		m_height = TFTHEIGHT;
		break;
	case 1:
	case 3:
		m_width = TFTHEIGHT;
		m_height = TFTWIDTH;
		break;
	}
	LCD_CS_ACTIVE();
#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
    uint16_t t;
    switch(m_rotation) {
		default: t = 0x1030; break;
		case 1: t = 0x1028; break;
		case 2: t = 0x1000; break;
		case 3: t = 0x1018; break;
    }
    LCD_Write16Register16(ILI932X_ENTRY_MOD, t); // MADCTL
    // For 932X, init default full-screen address window:
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(ILI9340) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520)
    uint8_t t;
    switch (m_rotation) {
		default: t = ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR; break;
		case 1: t = ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
		case 2: t = ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR; break;
		case 3:	t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
    }
    LCD_Write8Register8(ILI9341_MADCTL, t); // MADCTL
    // For 9341, init default full-screen address window:
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(ILI9340_INV) 
	uint8_t t;
    switch (m_rotation) {
		default: t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR; break;
		case 1: t = ILI9341_MADCTL_MV | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR; break;
		case 2: t = ILI9341_MADCTL_BGR; break;
		case 3:	t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
    }
    LCD_Write8Register8(ILI9341_MADCTL, t); // MADCTL
    // For 9341, init default full-screen address window:
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(UNKNOWN1602)
	uint8_t t;
	switch (m_rotation) {
		default: t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR; break;
		case 1: t = ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
		case 2: t = ILI9341_MADCTL_BGR; break;
		case 3: t = ILI9341_MADCTL_MX | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR; break;
	}
	LCD_Write8Register8(ILI9341_MADCTL, t); // MADCTL
	// For 9341, init default full-screen address window:
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(HX8347D) || defined(HX8347G)
    uint8_t t;
	switch(m_rotation) {
		default: t = 0; break;
		case 1: t = 0x60; break;
		case 2: t = 0xc0; break;
		case 3: t = 0xa0; break;
	}
	LCD_Write8Register8(HX8347G_MEMACCESS, t);
	/*
		7575 has to set the address window on most drawing operations.
		drawPixel() cheats by setting only the top left...by default,
		the lower right is always reset to the corner.
	*/
	LCD_SetLR();// CS_IDLE happens here
#elif defined(HX8357D)
    uint8_t t;
    switch (m_rotation) {
		default: t = HX8357B_MADCTL_MX | HX8357B_MADCTL_MY | HX8357B_MADCTL_RGB;	break;
		case 1:	t = HX8357B_MADCTL_MY | HX8357B_MADCTL_MV | HX8357B_MADCTL_RGB;	break;
		case 2: t = HX8357B_MADCTL_RGB;	break;
		case 3:	t = HX8357B_MADCTL_MX | HX8357B_MADCTL_MV | HX8357B_MADCTL_RGB;	break;
    }
    LCD_Write8Register8(HX8357_MADCTL, t); // MADCTL
    // For 8357, init default full-screen address window:
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#elif defined(SSD1297)
    uint16_t t1, t2;
	switch (m_rotation) {
	default:
		t1 = SSD1297_DRVOUTCTL_TB | SSD1297_DRVOUTCTL_REV | SSD1297_DRVOUTCTL_BGR | 0x013F;
		t2 = SSD1297_ENTRYMODE1_ID3 | 0x4C00;
		break;
	case 1:
		t1 = SSD1297_DRVOUTCTL_TB | SSD1297_DRVOUTCTL_RL | SSD1297_DRVOUTCTL_REV | SSD1297_DRVOUTCTL_BGR | 0x013F;
		t2 = SSD1297_ENTRYMODE1_AM | SSD1297_ENTRYMODE1_ID3 | 0x4C00;
		break;
	case 2:
		t1 = SSD1297_DRVOUTCTL_RL | SSD1297_DRVOUTCTL_REV | SSD1297_DRVOUTCTL_BGR | 0x013F;
		t2 = SSD1297_ENTRYMODE1_ID3 | 0x4C00;
		break;
	case 3:
		t1 = SSD1297_DRVOUTCTL_REV | SSD1297_DRVOUTCTL_BGR | 0x013F;
		t2 = SSD1297_ENTRYMODE1_AM | SSD1297_ENTRYMODE1_ID3 | 0x4C00;
		break;
	}
	LCD_Write16Register8(SSD1297_DRVOUTCTL, t1); // DRVOUT
	LCD_Write16Register8(SSD1297_ENTRYMODE1, t2); // ENTRYMODE
    LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
#endif
	LCD_CS_IDLE();
}

/**
 * \brief  Draws a BMP picture loaded in the STM32 MCU internal memory.
 *
 * \param xPos		Bmp X position in the LCD
 * \param yPos		Bmp Y position in the LCD
 * \param pBmp		Pointer to Bmp picture address
 * \return void
 */
void LCD_DrawBMP(int16_t xPos, int16_t yPos, const uint8_t *pBmp) {
	uint8_t *ptr;
	uint8_t *start;
	uint8_t *end;
	uint32_t offset = 0, size = 0;
	int32_t height = 0, width = 0;
	uint16_t colordepth = 0;

	/* Read bitmap size */
	size = *(volatile uint16_t *) (pBmp + 2);
	size |= (*(volatile uint16_t *) (pBmp + 4)) << 16;
	/* Get bitmap data address offset */
	offset = *(volatile uint16_t *) (pBmp + 10);
	offset |= (*(volatile uint16_t *) (pBmp + 12)) << 16;
	/* Read bitmap width */
	width = *(uint16_t *) (pBmp + 18);
	width |= (*(uint16_t *) (pBmp + 20)) << 16;
	/* Read bitmap height */
	height = *(uint16_t *) (pBmp + 22);
	height |= (*(uint16_t *) (pBmp + 24)) << 16;
	/* Read color depth */
	colordepth = *(uint16_t *) (pBmp + 28);

	/* Calculate pixel data boundary addresses */
	start = (uint8_t *) pBmp + offset;
	end = (uint8_t *) pBmp + size;

	/* Start drawing */
	if ((xPos + width >= m_width) || (yPos + abs(height) >= m_height)) return;
	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + abs(height) - 1);
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
	LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(ILI932X_GRAM_WR); // Write data to GRAM
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520) || defined(UNKNOWN1602)
	LCD_Write8(ILI9341_MEMORYWRITE); // Write data to GRAM
#elif defined(HX8347D) || defined(HX8347G)
	LCD_Write8(HX8347G_SRAM_WR); // Write data to GRAM
#elif defined(HX8357D)
	LCD_Write8(HX8357_RAMWR); // Write data to GRAM
#elif defined(SSD1297)
	LCD_Write8(SSD1297_RAMDATA_WRITE); // Write data to GRAM
#endif
	LCD_CD_DATA();
	if (height < 0) {
		/* Top-bottom file */
		ptr = start;
		/* Draw image */
		if (colordepth == 16) {
			while (ptr < end) {
#if defined(SSD1297)
				LCD_Write8(LCD_Color565_to_R(*((uint16_t *) ptr)));
				LCD_Write8(LCD_Color565_to_G(*((uint16_t *) ptr)));
				LCD_Write8(LCD_Color565_to_B(*((uint16_t *) ptr)));
#else
				LCD_Write8(*(ptr + 1));
				LCD_Write8(*(ptr));
#endif
				ptr += 2;
			}
		} else if (colordepth == 24) {
			while (ptr < end) {
#if defined(SSD1297)
				LCD_Write8(*(ptr + 2));
				LCD_Write8(*(ptr + 1));
				LCD_Write8(*(ptr));
#else
				LCD_Write8((*(ptr + 2) & 0xF8) | (*(ptr + 1) >> 5));
				LCD_Write8(((*(ptr + 1) & 0x1C) << 3) | (*(ptr) >> 3));
#endif
				ptr += 3;
			}
		}
	} else {
		/* Bottom-top file */
		uint8_t *rowstart;
		uint8_t *rowend;
		/* Draw image */
		if (colordepth == 16) {
			for (uint16_t row = height - 1; row > 0; row--) {
				rowstart = start + row * width * 2;
				rowend = start + (row + 1) * width * 2 - 1;
				ptr = rowstart;
				while (ptr < rowend) {
#if defined(SSD1297)
					LCD_Write8(LCD_Color565_to_R(*((uint16_t *) ptr)));
					LCD_Write8(LCD_Color565_to_G(*((uint16_t *) ptr)));
					LCD_Write8(LCD_Color565_to_B(*((uint16_t *) ptr)));
#else
					LCD_Write8(*(ptr + 1));
					LCD_Write8(*(ptr));
#endif
					ptr += 2;
				}
			}
		} else if (colordepth == 24) {
			for (uint16_t row = height - 1; row > 0; row--) {
				rowstart = start + row * width * 3;
				rowend = start + (row + 1) * width * 3 - 1;
				ptr = rowstart;
				while (ptr < rowend) {
#if defined(SSD1297)
					LCD_Write8(*(ptr + 2));
					LCD_Write8(*(ptr + 1));
					LCD_Write8(*(ptr));
#else
					LCD_Write8((*(ptr + 2) & 0xF8) | (*(ptr + 1) >> 5));
					LCD_Write8(((*(ptr + 1) & 0x1C) << 3) | (*(ptr) >> 3));
#endif
					ptr += 3;
				}
			}
		}
	}
	LCD_CS_IDLE();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}

#if defined(USE_FATFS)
/**
 * \brief  Draws a bitmap picture from FatFs file.
 *
 * \param  xPos: Bmp X position in the LCD
 * \param  yPos: Bmp Y position in the LCD
 * \param  pFile: Pointer to FIL object with bmp picture
 * \retval None
 */
void LCD_DrawBMPFromFile(int16_t xPos, int16_t yPos, FIL * pFile) {
	uint8_t buf[TFTWIDTH * 3]; // max 240 pixels at a time (720B)
	uint32_t readBytes = 0, clusterSize = 0, clusterNum = 0, clusterTotal = 0;
	uint8_t* pBmp;
	uint8_t* start;
	uint8_t* end;
	uint32_t offset = 0, size = 0;
	int32_t height = 0, width = 0;
	uint16_t colordepth = 0;

	/* Read BMP header: 54 bytes = 14 bytes header + 40 bytes DIB header (assuming BITMAPINFOHEADER) */
	f_read(pFile, buf, 54, (UINT *) &readBytes);
	if (readBytes != 54) {
		return;
	}
	pBmp = buf;
	/* Read bitmap size */
	size = *(volatile uint16_t *) (pBmp + 2);
	size |= (*(volatile uint16_t *) (pBmp + 4)) << 16;
	/* Get bitmap data address offset */
	offset = *(volatile uint16_t *) (pBmp + 10);
	offset |= (*(volatile uint16_t *) (pBmp + 12)) << 16;
	/* Read bitmap width */
	width = *(uint16_t *) (pBmp + 18);
	width |= (*(uint16_t *) (pBmp + 20)) << 16;
	/* Read bitmap height */
	height = *(uint16_t *) (pBmp + 22);
	height |= (*(uint16_t *) (pBmp + 24)) << 16;
	/* Read color depth */
	colordepth = *(uint16_t *) (pBmp + 28);

	/* Calculate total number of clusters to read */
	clusterSize = width * colordepth / 8;
	clusterTotal = abs(height);

	/* Start drawing */
	if ((xPos + width > m_width) || (yPos + abs(height) > m_height) || clusterSize > sizeof(buf)) return;
	LCD_SetAddrWindow(xPos, yPos, xPos + width - 1, yPos + abs(height) - 1);
	LCD_CS_ACTIVE();
	LCD_CD_COMMAND();
#if	defined(ILI9325) || defined(ILI9328) || defined(R61505) || defined(R61505V) || defined(S6D0154)
	LCD_Write8(0x00); // High byte of GRAM register...
	LCD_Write8(ILI932X_GRAM_WR); // Write data to GRAM
#elif defined(ILI9340) || defined(ILI9340_INV) || defined(ILI9341) || defined(ILI9341_00) || defined(R61520) || defined(UNKNOWN1602)
	LCD_Write8(ILI9341_MEMORYWRITE); // Write data to GRAM
#elif defined(HX8347D) || defined(HX8347G)
	LCD_Write8(HX8347G_SRAM_WR); // Write data to GRAM
#elif defined(HX8357D)
	LCD_Write8(HX8357_RAMWR); // Write data to GRAM
#elif defined(SSD1297)
	LCD_Write8(SSD1297_RAMDATA_WRITE); // Write data to GRAM
#endif

	LCD_CD_DATA();
	if (height < 0) {
		/* Top-bottom file */
		/* Move read pointer to beginning of pixel data */
		f_lseek(pFile, offset);
		clusterNum = 0;
		while (clusterNum <= clusterTotal) {
			/* Read new cluster */
			f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
			start = buf;
			end = buf + readBytes;
			pBmp = start;
			/* Draw image */
			if (colordepth == 16) {
				while (pBmp < end) {
#if defined(SSD1297)
					LCD_Write8(LCD_Color565_to_R(*((uint16_t *) pBmp)));
					LCD_Write8(LCD_Color565_to_G(*((uint16_t *) pBmp)));
					LCD_Write8(LCD_Color565_to_B(*((uint16_t *) pBmp)));
#else
					LCD_Write8(*(pBmp + 1));
					LCD_Write8(*(pBmp));
#endif
					pBmp += 2;
				}
			} else if (colordepth == 24) {
				while (pBmp < end) {
#if defined(SSD1297)
					LCD_Write8(*(pBmp + 2));
					LCD_Write8(*(pBmp + 1));
					LCD_Write8(*(pBmp));
#else
					LCD_Write8((*(pBmp + 2) & 0xF8) | (*(pBmp + 1) >> 5));
					LCD_Write8(((*(pBmp + 1) & 0x1C) << 3) | (*(pBmp) >> 3));
#endif
					pBmp += 3;
				}
			}
			clusterNum++;
		}
	} else {
		/* Bottom-top file */
		clusterNum = clusterTotal;
		while (clusterNum > 0) {
			f_lseek(pFile, offset + (clusterNum - 1) * clusterSize);
			f_read(pFile, buf, clusterSize, (UINT *) &readBytes);
			start = buf;
			end = buf + readBytes;
			pBmp = start;
			if (colordepth == 16) {
				while (pBmp < end) {
#if defined(SSD1297)
					LCD_Write8(LCD_Color565_to_R(*((uint16_t *) (pBmp))));
					LCD_Write8(LCD_Color565_to_G(*((uint16_t *) (pBmp))));
					LCD_Write8(LCD_Color565_to_B(*((uint16_t *) (pBmp))));
#else
					LCD_Write8(*(pBmp + 1));
					LCD_Write8(*(pBmp));
#endif
					pBmp += 2;
				}
			} else if (colordepth == 24) {
				while (pBmp < end) {
#if defined(SSD1297)
					LCD_Write8(*(pBmp + 2));
					LCD_Write8(*(pBmp + 1));
					LCD_Write8(*(pBmp));
#else
					LCD_Write8((*(pBmp + 2) & 0xF8) | (*(pBmp + 1) >> 5));
					LCD_Write8(((*(pBmp + 1) & 0x1C) << 3) | (*(pBmp) >> 3));
#endif
					pBmp += 3;
				}
			}
			clusterNum--;
		}
	}
	LCD_CS_IDLE();
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
}
#endif

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
void LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
	// Bresenham's algorithm - thx wikpedia

	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			LCD_DrawPixel(y0, x0, color);
		} else {
			LCD_DrawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
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
	
	// Initial off-screen clipping
	if ((length <= 0) || (y < 0) || (y >= m_height) || (x >= m_width) || 
		((x2 = (x + length - 1)) < 0)) return;

	if (x < 0) { // Clip left
		length += x;
		x = 0;
	}

	if (x2 >= m_width) { // Clip right
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
void LCD_DrawFastVLine(int16_t x, int16_t y, int16_t length, uint16_t color) {
	int16_t y2;

	// Initial off-screen clipping
	if ((length <= 0) || (x < 0) || (x >= m_width) || (y >= m_height) || 
		((y2 = (y+length-1)) <  0)) return;
		
	if (y < 0) { // Clip top
		length += y;
		y = 0;
	}
	if (y2 >= m_height) { // Clip bottom
		y2 = m_height - 1;
		length = y2 - y + 1;
	}
	LCD_SetAddrWindow(x, y, x, y2);
	LCD_Flood(color, length);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
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
void LCD_FillRect(int16_t x, int16_t y1, int16_t w, int16_t h, uint16_t color) {
	int16_t x2, y2;

	// Initial off-screen clipping
	if ((w <= 0) || (h <= 0) || (x >= m_width) || (y1 >= m_height)
			|| ((x2 = x + w - 1) < 0) || ((y2 = y1 + h - 1) < 0))
		return;
	if (x < 0) { // Clip left
		w += x;
		x = 0;
	}
	if (y1 < 0) { // Clip top
		h += y1;
		y1 = 0;
	}
	if (x2 >= m_width) { // Clip right
		x2 = m_width - 1;
		w = x2 - x + 1;
	}
	if (y2 >= m_height) { // Clip bottom
		y2 = m_height - 1;
		h = y2 - y1 + 1;
	}

	LCD_SetAddrWindow(x, y1, x2, y2);
	LCD_Flood(color, (uint32_t) w * (uint32_t) h);
	LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
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
void LCD_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
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
void LCD_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
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
void LCD_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	LCD_DrawLine(x0, y0, x1, y1, color);
	LCD_DrawLine(x1, y1, x2, y2, color);
	LCD_DrawLine(x2, y2, x0, y0, color);
}

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
void LCD_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}
	if (y1 > y2) {
		swap(y2, y1); 
		swap(x2, x1);
	}
	if (y0 > y1) {
		swap(y0, y1); 
		swap(x0, x1);
	}

	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if(x1 < a)      a = x1;
		else if(x1 > b) b = x1;
		if(x2 < a)      a = x2;
		else if(x2 > b) b = x2;
		LCD_DrawFastHLine(a, y0, b - a + 1, color);
		return;
	}

	int16_t	dx01 = x1 - x0,
			dy01 = y1 - y0,
			dx02 = x2 - x0,
			dy02 = y2 - y0,
			dx12 = x2 - x1,
			dy12 = y2 - y1;
	int32_t	sa   = 0,
			sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2) last = y1;   // Include y1 scanline
	else last = y1 - 1; // Skip it

	for(y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) swap(a,b);
		LCD_DrawFastHLine(a, y, b-a+1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if(a > b) swap(a,b);
		LCD_DrawFastHLine(a, y, b - a + 1, color);
	}
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
void LCD_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
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
void LCD_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
	// smarter version
	LCD_FillRect(x + r, y, w - 2 * r, h, color);

	// draw four corners
	LCD_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	LCD_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
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
void LCD_DrawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t fontindex) {
	uint16_t height, width, bytes;
	uint8_t offset;
	uint32_t charindex = 0;
	uint8_t *pchar;
	uint32_t line = 0;

	height = fonts[fontindex]->Height;
	width = fonts[fontindex]->Width;

	if ((x >= m_width) || // Clip right
		(y >= m_height) || // Clip bottom
		((x + width - 1) < 0) || // Clip left
		((y + height - 1) < 0))   // Clip top
		return;

	bytes = (width + 7) / 8;
	if (c < ' ') c = ' ';
#ifndef USE_CP1251
	else if (c > '~') c = ' ';
#endif
	charindex = (c - ' ') * height * bytes;
	offset = 8 * bytes - width;

	for (uint32_t i = 0; i < height; i++) {
		pchar = ((uint8_t *) &fonts[fontindex]->table[charindex] + (width + 7) / 8 * i);
		switch (bytes) {
		case 1:
			line = pchar[0];
			break;
		case 2:
			line = (pchar[0] << 8) | pchar[1];
			break;
		case 3:
		default:
			line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
			break;
		}
		for (uint32_t j = 0; j < width; j++) {
			if (line & (1 << (width - j + offset - 1))) {
				LCD_DrawPixel((x + j), y, color);
			} else {
				LCD_DrawPixel((x + j), y, bg);
			}
		}
		y++;
	}
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
	
	volatile uint16_t height, width;
	height = fonts[m_font]->Height;
	width = fonts[m_font]->Width;

	p = buf;
	while (*p) {
		if (*p == '\n') {
			m_cursor_y += height;
			m_cursor_x = 0;
		} else if (*p == '\r') {
			m_cursor_x = 0;
		} else if (*p == '\t') {
			m_cursor_x += width * 4;
		} else {
#ifdef WIPE_LINES
			if (m_cursor_x == 0) {
				LCD_SetAddrWindow(0, m_cursor_y, m_width - 1, m_cursor_y + height);
				LCD_Flood(m_textbgcolor, (long) m_width * height);
				LCD_SetAddrWindow(0, 0, m_width - 1, m_height - 1);
			}
#endif
			if (m_cursor_y >= (m_height - height)) {
				m_cursor_y = 0;
#ifdef WIPE_SCREEN
				LCD_FillScreen(m_textbgcolor);
#endif
			}
			LCD_DrawChar(m_cursor_x, m_cursor_y, *p, m_textcolor, m_textbgcolor, m_font);
			m_cursor_x += width;
			if (m_wrap && (m_cursor_x > (m_width - width))) {
				m_cursor_y += height;
				m_cursor_x = 0;
			}
		}
		p++;
	}
}

/**
 * \brief Sets the cursor coordinates
 *
 * \param x		The x-coordinate
 * \param y		The y-coordinate
 *
 * \return void
 */
void LCD_SetCursor(uint16_t x, uint16_t y) {
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
	if (s < 0) {
		m_font = 0;
	} else if (s >= fontsNum) {
		m_font = fontsNum - 1;
	} else {
		m_font = s;
	}
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
 * \brief Get display rotation
 *
 * \param
 *
 * \return uint8_t rotation
 */
uint8_t LCD_GetRotation() {
	return m_rotation;
}

/**
 * \brief Gets the cursor x-coordinate
 *
 * \param 		
 *
 * \return int16_t x-coordinate
 */
int16_t LCD_GetCursorX(void) {
	return m_cursor_x;
}

/**
 * \brief Gets the cursor Y-coordinate
 *
 * \param 		
 *
 * \return int16_t y-coordinate
 */
int16_t LCD_GetCursorY(void) {
	return m_cursor_y;
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

inline uint32_t LCD_Color565_to_888(uint16_t color) {
    return (((color & 0xF800) << 8) | ((color & 0x7E0) << 5) | ((color & 0x1F) << 3));  // transform to rrrrrrxx ggggggxx bbbbbbxx
}

inline uint8_t LCD_Color565_to_R(uint16_t color) {
    return ((color & 0xF800) >> 8);  // transform to rrrrrrxx
}
inline uint8_t LCD_Color565_to_G(uint16_t color) {
    return ((color & 0x7E0) >> 3);  // transform to ggggggxx
}
inline uint8_t LCD_Color565_to_B(uint16_t color) {
    return ((color & 0x1F) << 3);  // transform to bbbbbbxx
}
