#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>

typedef struct
{    
  const uint8_t *table;
  uint16_t Width;
  uint16_t Height;
  
} font_t;

#define USE_FONT8
#define USE_FONT12
#define USE_FONT16
#define USE_FONT20
#define USE_FONT24

//#define USE_CP1251

#ifdef USE_FONT8
extern font_t Font8;
#endif
#ifdef USE_FONT12
extern font_t Font12;
#endif
#ifdef USE_FONT16
extern font_t Font16;
#endif
#ifdef USE_FONT20
extern font_t Font20;
#endif
#ifdef USE_FONT24
extern font_t Font24;
#endif

#if !(defined(USE_FONT8) || defined(USE_FONT12) || defined(USE_FONT16) || defined(USE_FONT20) || defined(USE_FONT24))
#error Please select your LCD font in fonts.h, lines 14-18
#endif

#endif /* __FONTS_H */
