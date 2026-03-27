#include "sine_gen.h"

#include <math.h>

#include "driver/dac_continuous.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void
generate_points(u8 *buffer, u32 freq, u8 ampl) {
    for(int i = 0; i < SINE_GEN_BUFFER_SIZE; i++) {
        float x = i * 2 * M_PI / SINE_GEN_BUFFER_SIZE;
        buffer[i] = 127.0f + ampl * sinf(x) + 0.5f;  // 0.5f so it rounds to the nearest integer value
    }
}

static inline bool
verify_params(u32 freq, u8 ampl) {
    return freq > 10 && ampl > 0 && ampl < 128;
}

esp_err_t
sine_gen_init(sine_gen_t *gen, u32 freq, u8 ampl) {
    if(!verify_params(freq, ampl)) {
        return ESP_ERR_INVALID_ARG;
    }

    gen->freq = freq;
    gen->ampl = ampl;

    // generate the sine wave points
    generate_points(gen->buffer, freq, ampl);

    dac_continuous_config_t conf = {
            .chan_mask = DAC_CHANNEL_MASK_CH0,
            // these are by default such, may need to change them
            .desc_num = 8,
            .buf_size = 2048,
            .freq_hz = SINE_GEN_BUFFER_SIZE * gen->freq,
            .offset = 0,
            // TODO: try out different clock setting to get around the minimum freq
            .clk_src = DAC_DIGI_CLK_SRC_APLL,
    };

    esp_err_t err = ESP_OK;
    err = dac_continuous_new_channels(&conf, &gen->dac_handle);
    if(err) {
        return err;
    }

    err = dac_continuous_enable(gen->dac_handle);
    if(err) {
        dac_continuous_del_channels(gen->dac_handle);
        return err;
    }

    err = dac_continuous_write_cyclically(gen->dac_handle, gen->buffer, SINE_GEN_BUFFER_SIZE, NULL);
    if(err) {
        dac_continuous_disable(gen->dac_handle);
        dac_continuous_del_channels(gen->dac_handle);
        return err;
    }

    return ESP_OK;
}

void
sine_gen_deinit(sine_gen_t *gen) {
    dac_continuous_disable(gen->dac_handle);
    dac_continuous_del_channels(gen->dac_handle);

    gpio_reset_pin(25);
}
