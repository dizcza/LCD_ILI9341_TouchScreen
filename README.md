# ILI9341 TFTLCD Touch Screen driver

Based on HAL for STM32 Nucleo F446RE. If you has a driver, different from ILI9341, refer to [`lcd.h`](TrueSTUDIO/F446_LCD_Touch/display/lcd.h).

You can import this project in Atollic TrueStudio, Ubuntu.

### LCD Touch pinout

 *   Y- PB10
 *   Y+ PA4, EXTI4
 *   X- PA1
 *   X+ PA8

In idle state, PA4 is in interrupt mode awaiting for touches. You can't draw and await for touches simultaneously.

### Measuring X-pos

* set PA1 to logical 0
* set PA8 to logical 1
* set PB10 in input mode
* set PA4 in analog mode
* get value from ADC2 channel 4


### Measuring Y-pos

* disable EXTI4 line
* set PB10 to logical 0
* set PA4 to logical 1
* set PA8 in input mode
* set PA1 in analog mode
* get value from ADC1 channel 1
* enable EXTI4 line


### How to use
```c
// initialize ADC1 & ADC2

LCD_Init();
LCD_Touch_Init(&hadc2, ADC_CHANNEL_4, &hadc1, ADC_CHANNEL_1);
LCD_SetMode(TOUCH);
LCD_TouchPoint p;

while (1) {
    LCD_Touch_Read(&p);  // read raw touch point
    // do whatever you want with the touch point `p`
}
```

`LCD_TouchPoint` is a struct that holds X and Y position, time, and touch state that could be one of `LCD_TOUCH_DOWN`, `LCD_TOUCH_MOVE`, or `LCD_TOUCH_UP`.

### YouTube Demo

[![Demo CountPages alpha](http://i64.tinypic.com/e9svua.png)](https://www.youtube.com/watch?v=3GNu_ODub5Q&feature=youtu.be)
