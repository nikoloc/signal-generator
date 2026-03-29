#ifndef SINE_GEN_H
#define SINE_GEN_H

#include "driver/dac_cosine.h"
#include "gen.h"

// for sine generation instead of using the generic `dac_dma_gen_t` we use a hardware based sine generator, which
// provides better results
typedef struct sine_gen {
    gen_t base_gen;

    dac_cosine_handle_t dac_handle;
} sine_gen_t;

void
sine_gen_init(sine_gen_t *gen);

#endif
