#include "main.h"

#include "ctl.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "keypad.h"
#include "lcd.h"
#include "soc/soc.h"
#include "ui.h"

const char *TAG = "MAIN";

static void
init_display(void) {
    lcd_config_t lcd_conf = {
            .enable = O_LCD_ENABLE,
            .reg_select = O_LCD_REG_SELECT,
            .data_pins =
                    {
                            O_LCD_DATA_PIN_0,
                            O_LCD_DATA_PIN_1,
                            O_LCD_DATA_PIN_2,
                            O_LCD_DATA_PIN_3,
                    },
    };

    ESP_ERROR_CHECK(lcd_tinit(&lcd_conf, PRO_CPU_NUM));

    vTaskDelay(pdMS_TO_TICKS(100));
    // render the initial scene
    ui_render();
}

void
app_main(void) {
    ESP_LOGI(TAG, "initilizing the generator...");

    ESP_LOGI(TAG, "initilizing the controler...");
    ctl_init();

    ESP_LOGI(TAG, "initilizing the ui...");
    ui_init();

    ESP_LOGI(TAG, "initilizing the display...");
    init_display();

    ESP_LOGI(TAG, "initilizing the keypad...");
    keypad_init();

    ESP_LOGI(TAG, "initilizition successful!");

    // nothing, just wait
    while(1) {
        vTaskDelay(1000);
    }
}
