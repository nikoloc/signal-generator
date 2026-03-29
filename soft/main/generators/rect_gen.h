#ifndef RECT_GEN_H
#define RECT_GEN_H

#include "gen.h"

// for rectangle generation instead of using the generic `dac_dma_gen_t` we use a hardware based pwm controller, which
// provides better results
typedef struct rect_gen {
    gen_t base_gen;
} rect_gen_t;

void
rect_gen_init(rect_gen_t *gen);

#endif
