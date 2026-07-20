#include "none_gen.h"

static esp_err_t
none_gen_start(gen_t *base_gen, int freq, float symmetry) {
    return ESP_OK;
}

static esp_err_t
none_gen_stop(gen_t *base_gen) {
    return ESP_OK;
}

static const gen_interface_t none_gen_impl = {
        .start = none_gen_start,
        .stop = none_gen_stop,
};

void
none_gen_init(none_gen_t *gen) {
    gen_init(&gen->base_gen, &none_gen_impl);
}
