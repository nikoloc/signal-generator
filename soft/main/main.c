#include "main.h"

#include "ctl.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "keypad.h"
#include "lcd.h"
#include "ui.h"

static const char *TAG = "MAIN";

void
app_main(void) {
    ESP_LOGI(TAG, "initilizing the generator...");

    ESP_LOGI(TAG, "initilizing the controler...");
    ctl_init();

    ESP_LOGI(TAG, "initilizing the display...");
    lcd_init();

    ESP_LOGI(TAG, "initilizing the ui...");
    ui_init();

    ESP_LOGI(TAG, "initilizing the keypad...");
    keypad_init();

    ESP_LOGI(TAG, "initilizition successful!");

    // nothing, just wait
    while(1) {
        vTaskDelay(1000);
    }
}
