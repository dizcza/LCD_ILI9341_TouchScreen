/*
 * lcd_touch_draw.c
 *
 *  Created on: Mar 7, 2019
 *      Author: dizcza
 */


#define LCD_TOUCH_DRAW_POINT_RADIUS  3

#include "lcd_touch.h"

static LCD_TouchPoint m_last_touch_point = {
		.x=0,
		.y=0,
		.tick=0,
		.state=LCD_TOUCH_IDLE
};

static void DrawTouchPoint(const LCD_TouchPoint* p) {
	if (p->state == LCD_TOUCH_DOWN) {
		LCD_FillCircle(p->x, p->y, LCD_TOUCH_DRAW_POINT_RADIUS, GREEN);
	} else if (p->state == LCD_TOUCH_UP) {
		LCD_FillCircle(p->x, p->y, LCD_TOUCH_DRAW_POINT_RADIUS, RED);
	}
}

void LCD_Touch_Draw_ConnectLastPoint(const LCD_TouchPoint* p) {
	if (m_last_touch_point.state == LCD_TOUCH_DOWN || m_last_touch_point.state == LCD_TOUCH_MOVE) {
		// connect two last points
		LCD_SetMode(LCD_MODE_DRAW);
		LCD_DrawLine(m_last_touch_point.x, m_last_touch_point.y, p->x, p->y, WHITE);
		DrawTouchPoint(&m_last_touch_point);
		DrawTouchPoint(p);
		LCD_SetMode(LCD_MODE_TOUCH);
	}
	m_last_touch_point.x = p->x;
	m_last_touch_point.y = p->y;
	m_last_touch_point.tick = p->tick;
	m_last_touch_point.state = p->state;
}

void LCD_Touch_Draw_PrintInfo() {
	if (m_last_touch_point.state == LCD_TOUCH_IDLE) {
		// no touch is made
		return;
	}
	LCD_SetMode(LCD_MODE_DRAW);
	LCD_SetCursor(0, 0);
	LCD_Printf("Last touch: x=%3d y=%3d\n", m_last_touch_point.x, m_last_touch_point.y);
	switch (m_last_touch_point.state) {
	case LCD_TOUCH_DOWN:
		LCD_Printf("LCD_TOUCH_DOWN\n");
		break;
	case LCD_TOUCH_MOVE:
		LCD_Printf("LCD_TOUCH_MOVE\n");
		break;
	case LCD_TOUCH_UP:
		LCD_Printf("LCD_TOUCH_UP  \n");
		break;
	default:
		// should never be here
		LCD_Printf("(invalid touch state)\n");
		break;
	}
	//FIXME starts constant TOUCH_UP interrupts
	LCD_SetMode(LCD_MODE_TOUCH);
}

void LCD_Touch_Draw_OnUp() {
	m_last_touch_point.state = LCD_TOUCH_UP;
	LCD_SetMode(LCD_MODE_DRAW);
	DrawTouchPoint(&m_last_touch_point);
	LCD_SetMode(LCD_MODE_TOUCH);
	LCD_Touch_Draw_PrintInfo();
}
