#include "gen.h"

#include "driver/gpio.h"
#include "main.h"

void
gen_init(gen_t *gen, const gen_interface_t *impl) {
    gen->active = false;
    gen->impl = impl;
}

esp_err_t
gen_start(gen_t *gen, gen_params_t *params) {
    esp_err_t err = gen->impl->start(gen, params);
    if(err) {
        return err;
    }

    gen->active = true;
    return ESP_OK;
}

esp_err_t
gen_stop(gen_t *gen) {
    gen->active = false;
    esp_err_t err = gen->impl->stop(gen);
    if(err) {
        return err;
    }

    // ground the pin so there is no residue output
    err = gpio_reset_pin(O_SIGNAL);
    if(err) {
        return err;
    }

    err = gpio_set_direction(O_SIGNAL, GPIO_MODE_OUTPUT);
    if(err) {
        return err;
    }
    return gpio_set_level(O_SIGNAL, 0);
}
