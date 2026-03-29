#include "sine_gen.h"

#include "util/macros.h"

static esp_err_t
sine_gen_start(gen_t *base_gen, gen_params_t *params) {
    sine_gen_t *gen = CONTAINER_OF(base_gen, sine_gen_t, base_gen);

    dac_cosine_config_t conf = {
            .chan_id = DAC_CHAN_0,
            .freq_hz = params->freq,
            .phase = DAC_COSINE_PHASE_0,
    };

    esp_err_t err = dac_cosine_new_channel(&conf, &gen->dac_handle);
    if(err) {
        return err;
    }

    return dac_cosine_start(gen->dac_handle);
}

static esp_err_t
sine_gen_stop(gen_t *base_gen) {
    sine_gen_t *gen = CONTAINER_OF(base_gen, sine_gen_t, base_gen);
    esp_err_t err = dac_cosine_stop(gen->dac_handle);
    if(err) {
        return err;
    }

    return dac_cosine_del_channel(gen->dac_handle);
}

static const gen_interface_t sine_gen_impl = {
        .start = sine_gen_start,
        .stop = sine_gen_stop,
};

void
sine_gen_init(sine_gen_t *gen) {
    gen_init(&gen->base_gen, &sine_gen_impl);
}
