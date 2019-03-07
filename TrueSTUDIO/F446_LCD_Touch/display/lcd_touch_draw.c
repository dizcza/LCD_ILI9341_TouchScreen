/*
 * lcd_touch_draw.c
 *
 *  Created on: Mar 7, 2019
 *      Author: dizcza
 */


#include "lcd_touch.h"

static LCD_TouchPoint m_touch_points[2];
static uint32_t m_num_touches = 0U;

void LCD_Touch_Draw_Reset() {
	m_num_touches = 0U;
}

void LCD_Touch_Draw_UpdateLastPoint(const LCD_TouchPoint* p) {
	LCD_TouchPoint* p_update = &m_touch_points[m_num_touches++ % 2];
	p_update->x = p->x;
	p_update->y = p->y;
	p_update->time = p->time;
}

int8_t LCD_Touch_Draw_DrawLastStroke() {
	if (m_num_touches < 2U) {
		return 1;  // not enough touches in a stroke
	}
	LCD_SetMode(LCD_MODE_DRAW);
	LCD_TouchPoint* p_curr = &m_touch_points[(m_num_touches - 1) % 2];
	LCD_TouchPoint* p_prev = &m_touch_points[m_num_touches % 2];
	LCD_DrawLine(p_prev->x, p_prev->y, p_curr->x, p_curr->y, WHITE);
	LCD_SetMode(LCD_MODE_TOUCH);
	return 0;
}

void LCD_Touch_Draw_PrintInfo() {
	LCD_SetMode(LCD_MODE_DRAW);
	LCD_SetCursor(0, 0);
	LCD_Printf("Stroke length: %5d\n", m_num_touches);
	if (m_num_touches > 0U) {
		LCD_TouchPoint* p_curr = &m_touch_points[(m_num_touches - 1) % 2];
		LCD_Printf("Last touch: %3d %3d\n", p_curr->x, p_curr->y);
	}
	//FIXME starts constant TOUCH_UP interrupts
	LCD_SetMode(LCD_MODE_TOUCH);
}
