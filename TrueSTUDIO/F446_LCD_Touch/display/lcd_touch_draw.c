/*
 * lcd_touch_draw.c
 *
 *  Created on: Mar 7, 2019
 *      Author: dizcza
 */


#define LCD_TOUCH_DRAW_POINT_RADIUS  3

#include "lcd_touch.h"

static LCD_TouchPoint m_touch_points[2];
static uint32_t m_num_touches = 0U;

static void DrawTouchPoint(const LCD_TouchPoint* p);

void LCD_Touch_Draw_OnUp() {
	if (m_num_touches > 0) {
		LCD_TouchPoint* p_last = &m_touch_points[(m_num_touches - 1) % 2];
		p_last->state = LCD_TOUCH_UP;
	}
}

void LCD_Touch_Draw_UpdateLastPoint(const LCD_TouchPoint* p) {
	LCD_TouchPoint* p_update = &m_touch_points[m_num_touches++ % 2];
	p_update->x = p->x;
	p_update->y = p->y;
	p_update->tick = p->tick;
	p_update->state = p->state;
}

static void DrawTouchPoint(const LCD_TouchPoint* p) {
	if (p->state == LCD_TOUCH_DOWN) {
		LCD_FillCircle(p->x, p->y, LCD_TOUCH_DRAW_POINT_RADIUS, GREEN);
	} else if (p->state == LCD_TOUCH_UP) {
		LCD_FillCircle(p->x, p->y, LCD_TOUCH_DRAW_POINT_RADIUS, RED);
	}
}

int8_t LCD_Touch_Draw_DrawLastStroke() {
	if (m_num_touches < 2U) {
		return 1;  // not enough touches in a stroke
	}
	LCD_SetMode(LCD_MODE_DRAW);
	LCD_TouchPoint* p_curr = &m_touch_points[(m_num_touches - 1) % 2];
	LCD_TouchPoint* p_prev = &m_touch_points[m_num_touches % 2];
	LCD_DrawLine(p_prev->x, p_prev->y, p_curr->x, p_curr->y, WHITE);
	DrawTouchPoint(p_prev);
	DrawTouchPoint(p_curr);
	LCD_SetMode(LCD_MODE_TOUCH);
	return 0;
}

void LCD_Touch_Draw_PrintInfo() {
	LCD_SetMode(LCD_MODE_DRAW);
	LCD_SetCursor(0, 0);
	LCD_Printf("Stroke size: %5d\n", m_num_touches);
	if (m_num_touches > 0U) {
		LCD_TouchPoint* p_curr = &m_touch_points[(m_num_touches - 1) % 2];
		LCD_Printf("Last touch: x=%3d y=%3d\n", p_curr->x, p_curr->y);
		switch (p_curr->state) {
		case LCD_TOUCH_DOWN:
			LCD_Printf("LCD_TOUCH_DOWN\n");
			break;
		case LCD_TOUCH_MOVE:
			LCD_Printf("LCD_TOUCH_MOVE\n");
			break;
		case LCD_TOUCH_UP:
			LCD_Printf("LCD_TOUCH_UP  \n");
			m_num_touches = 0U;
			break;
		case LCD_TOUCH_IDLE:
		default:
			// should never be here
			LCD_Printf("(invalid touch state)\n");
			break;
		}
	}
	//FIXME starts constant TOUCH_UP interrupts
	LCD_SetMode(LCD_MODE_TOUCH);
}
