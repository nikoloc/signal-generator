#include <stdarg.h>
#include <stdio.h>

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main.h"
#include "util/ints.h"
#include "util/macros.h"

#define I2C_ADDRESS 0x27

#define LCD_RS (1 << 0)
#define LCD_RW (1 << 1)
#define LCD_EN (1 << 2)
#define LCD_BACKLIGHT (1 << 3)

static const char *TAG = "LCD";

i2c_master_dev_handle_t handle;

static void
lcd_write_nibble(u8 nibble, u8 mode) {
    u8 data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    u8 buffer[2] = {data | LCD_EN, data & ~LCD_EN};

    ESP_ERROR_CHECK(i2c_master_transmit(handle, buffer, 2, -1));
}

static void
lcd_send(u8 value, u8 mode) {
    lcd_write_nibble(value & 0xF0, mode);
    lcd_write_nibble((value << 4) & 0xF0, mode);
}

static void
lcd_command(u8 cmd) {
    lcd_send(cmd, 0);
}

static void
lcd_write_char(char c) {
    lcd_send(c, LCD_RS);
}

void
lcd_clear(void) {
    lcd_command(0x01);
    vTaskDelay(pdMS_TO_TICKS(20));
}

void
lcd_goto(int row, int col) {
    ASSERT(col < 16 && row < 4);

    static const int row_offsets[] = {0x00, 0x40, 0x10, 0x50};
    lcd_command(0x80 | (col + row_offsets[row]));
}

void
lcd_write(const char *str) {
    while(*str) {
        lcd_write_char(*str++);
    }
}

void
lcd_printf(int row, int col, const char *fmt, ...) {
    char buffer[17] = {0};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    lcd_goto(row, col);
    lcd_write(buffer);
}

void
lcd_init(void) {
    i2c_master_bus_config_t bus_config = {
            .i2c_port = -1,
            .sda_io_num = O_LCD_SDA,
            .scl_io_num = O_LCD_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDRESS,
            .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &handle));

    lcd_write_nibble(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write_nibble(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write_nibble(0x30, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    lcd_write_nibble(0x20, 0);

    lcd_command(0x28);  // 4 bit mod, 4 lines
    lcd_command(0x0C);  // display on, cursor off
    lcd_command(0x06);  // auto-increment
    vTaskDelay(pdMS_TO_TICKS(50));

    lcd_clear();
}

// for timing testing purposes
// void
// app_main(void) {
//     lcd_init();
//
//     lcd_goto(0, 0);
//     lcd_write("Novi ESP-IDF API");
//
//     lcd_goto(1, 0);
//     lcd_write("Radi savrseno!");
//
//     vTaskDelay(pdMS_TO_TICKS(2000));
//     lcd_clear();
//
//     lcd_goto(2, 0);
//     lcd_write("Hello");
//
//     lcd_goto(2, 8);
//     lcd_write("Darko");
//
//     vTaskDelay(pdMS_TO_TICKS(2000));
//     lcd_clear();
//
//     lcd_printf(3, 0, "Hello, %s", "Darko");
// }
