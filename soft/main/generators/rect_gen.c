#include "rect_gen.h"

#include "driver/ledc.h"
#include "gen.h"
#include "util/macros.h"

static inline bool
verify_params(u32 freq, float duty) {
    return freq > 0 && duty >= 0 && duty <= 1;
}

static esp_err_t
rect_gen_start(gen_t *base_gen, gen_params_t *params) {
    if(!verify_params(params->freq, params->duty)) {
        return ESP_ERR_INVALID_ARG;
    }

    ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_num = LEDC_TIMER_0,
            .duty_resolution = LEDC_TIMER_8_BIT,
            .freq_hz = params->freq,
            .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timer_conf);
    if(err) {
        return err;
    }

    ledc_channel_config_t channel_conf = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .timer_sel = LEDC_TIMER_0,
            .gpio_num = 25,
            .duty = CLAMP(params->duty * params->duty, 0, 255),
            .hpoint = 0,
    };

    err = ledc_channel_config(&channel_conf);
    if(err) {
        return err;
    }

    return ESP_OK;
}

static esp_err_t
rect_gen_stop(gen_t *base_gen) {
    return ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
}

static const gen_interface_t rect_gen_impl = {
        .start = rect_gen_start,
        .stop = rect_gen_stop,
};

void
rect_gen_init(rect_gen_t *gen) {
    gen_init(&gen->base_gen, &rect_gen_impl);
}
