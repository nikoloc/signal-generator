#include "isr_mgr.h"

#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "util/ints.h"

static struct g {
    isr_callback_t callback;
    QueueHandle_t event_queue_handle;
    TaskHandle_t task_handle;
} g;

static void IRAM_ATTR
gpio_isr_handler(void *arg) {
    int pin = (int)arg;

    xQueueSendFromISR(g.event_queue_handle, &pin, NULL);
}

static void
task_handle_events(void *arg) {
    int pin;

    while(1) {
        if(xQueueReceive(g.event_queue_handle, &pin, portMAX_DELAY)) {
            g.callback(pin);
        }
    }
}

esp_err_t
isr_mgr_init(isr_callback_t callback) {
    g.callback = callback;

    esp_err_t err = ESP_FAIL;

    g.event_queue_handle = xQueueCreate(ISR_MGR_EVENT_QUEUE_SIZE, sizeof(int));
    if(!g.event_queue_handle) {
        goto err;
    }

    // start the task that will react to events, using the default values
    int success = xTaskCreate(task_handle_events, "handle_events", 2048, NULL, 10, &g.task_handle);
    if(!success) {
        goto err_queue;
    }

    err = gpio_install_isr_service(0);
    if(err) {
        goto err_task;
    }

    return ESP_OK;

err_task:
    vTaskDelete(g.task_handle);
err_queue:
    vQueueDelete(g.event_queue_handle);
err:
    return err;
}

esp_err_t
isr_mgr_add_source(int pin, gpio_int_type_t intr, pull_t pull) {
    gpio_config_t conf = {
            .mode = GPIO_MODE_INPUT,
            .pin_bit_mask = BIT(pin),
            .intr_type = intr,
            .pull_up_en = pull == PULL_UP,
            .pull_down_en = pull == PULL_DOWN,
    };

    esp_err_t err = gpio_config(&conf);
    if(err) {
        return err;
    }

    return gpio_isr_handler_add(pin, gpio_isr_handler, (void *)pin);
}
