#include "rect_gen.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

static inline bool
verify_params(u32 freq, u8 ampl, u8 duty) {
    return freq > 0 && ampl > 0 && ampl < 128 && duty > 0;
}

esp_err_t
rect_gen_init(rect_gen_t *gen, u32 freq, u8 ampl, u8 duty) {
    if(!verify_params(freq, ampl, duty)) {
        return ESP_ERR_INVALID_ARG;
    }

    gen->freq = freq;
    gen->ampl = ampl;
    gen->duty = duty;

    esp_err_t err = ESP_OK;

    ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = LEDC_TIMER_0,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = freq,
            .clk_cfg = LEDC_AUTO_CLK,
    };

    err = ledc_timer_config(&timer_conf);
    if(err) {
        return err;
    }

    ledc_channel_config_t channel_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .timer_sel = LEDC_TIMER_0,
            .gpio_num = 25,
            .duty = duty,
            .hpoint = 0,
    };

    err = ledc_channel_config(&channel_conf);
    if(err) {
        return err;
    }

    err = ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq);
    if(err) {
        return err;
    }

    return ESP_OK;
}

void
rect_gen_deinit(rect_gen_t *gen) {
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    gpio_reset_pin(25);
}
