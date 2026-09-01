#ifndef LCD_H
#define LCD_H

#include <driver/gpio.h>
#include <rom/ets_sys.h>
#include <stdint.h>

// all commands possible on an LCD display, left for reference, not used rn
#define LCD_CLEAR_CURSOR_HOME 0x01
#define LCD_CURSOR_HOME 0x02
#define LCD_CURSOR_DEC_SHIFT_OFF 0x04
#define LCD_CURSOR_DEC_SHIFT_ON 0x05
#define LCD_CURSOR_INC_SHIFT_OFF 0x06
#define LCD_CURSOR_INC_SHIFT_ON 0x07
#define LCD_DISPLAY_OFF_CURSOR_OFF_CURSOR_BLINK_OFF 0x08
#define LCD_DISPLAY_ON_CURSOR_OFF_CURSOR_BLINK_OFF 0x0C
#define LCD_DISPLAY_ON_CURSOR_ON_CURSOR_BLINK_OFF 0x0E
#define LCD_DISPLAY_ON_CURSOR_ON_CURSOR_BLINK_ON 0x0F
#define LCD_CURSOR_SHIFT_LEFT 0x10
#define LCD_CURSOR_SHIFT_RIGHT 0x14
#define LCD_DISPLAY_SHIFT_LEFT 0x18
#define LCD_DISPLAY_SHIFT_RIGHT 0x1C
#define LCD_CURSOR_TO_SECOND_ROW 0xC0

void
lcd_init(void);

void
lcd_clear(void);

void
lcd_goto(int col, int row);

void
lcd_write(const char *str);

void
lcd_printf(int row, int col, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#endif
