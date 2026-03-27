#ifndef ISR_MGR_H
#define ISR_MGR_H

#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "hal/gpio_types.h"

#define ISR_MGR_EVENT_QUEUE_SIZE 16

typedef void (*isr_callback_t)(int pin);

typedef enum pull {
    PULL_NONE = 0,
    PULL_UP,
    PULL_DOWN,
} pull_t;

esp_err_t
isr_mgr_init(isr_callback_t callback);

esp_err_t
isr_mgr_add_source(int pin, gpio_int_type_t intr, pull_t pull);

#endif
