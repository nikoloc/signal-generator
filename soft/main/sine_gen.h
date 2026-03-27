#ifndef SINE_GEN_H
#define SINE_GEN_H

#include "driver/dac_continuous.h"
#include "esp_err.h"
#include "util/ints.h"

#define SINE_GEN_BUFFER_SIZE 256

typedef struct sine_gen {
    u32 freq;
    u8 ampl;

    u8 buffer[SINE_GEN_BUFFER_SIZE];
    dac_continuous_handle_t dac_handle;
} sine_gen_t;

esp_err_t
sine_gen_init(sine_gen_t *gen, u32 freq, u8 ampl);

void
sine_gen_deinit(sine_gen_t *gen);

#endif
