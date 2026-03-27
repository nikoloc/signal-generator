#include "cosine_gen_hw.h"

#include "driver/dac_cosine.h"
#include "freertos/idf_additions.h"

void
cosine_gen_hw_example(void) {
    printf("running `cosine_gen_hw_example()`\n");

    dac_cosine_config_t conf = {
            .chan_id = DAC_CHAN_0,
            .freq_hz = 1000,
            .phase = DAC_COSINE_PHASE_0,
    };

    dac_cosine_handle_t handle_0;
    ESP_ERROR_CHECK(dac_cosine_new_channel(&conf, &handle_0));

    conf.chan_id = DAC_CHAN_1;
    conf.phase = DAC_COSINE_PHASE_180;

    dac_cosine_handle_t handle_1;
    ESP_ERROR_CHECK(dac_cosine_new_channel(&conf, &handle_1));

    // start them both back to back
    ESP_ERROR_CHECK(dac_cosine_start(handle_0));
    ESP_ERROR_CHECK(dac_cosine_start(handle_1));

    while(1) {
        vTaskDelay(100);
    }
}
