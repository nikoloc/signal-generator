/*
 * SPDX-FileCopyrightText: 2020-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "cosine_gen_hw.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "gen.h"
#include "hal/adc_types.h"
#include "soc/soc_caps.h"
#include "util/ints.h"
#include "util/macros.h"

static inline void
delay(u32 ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}
void
app_main(void) {
    // cosine_gen_hw_example();
    gen_setup_and_run();
}
