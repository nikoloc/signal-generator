#ifndef RECT_GEN_H
#define RECT_GEN_H

#include "esp_err.h"
#include "util/ints.h"

typedef struct rect_gen {
    u32 freq;
    u8 ampl;
    u8 duty;  // 0 - 255
} rect_gen_t;

esp_err_t
rect_gen_init(rect_gen_t *gen, u32 freq, u8 ampl, u8 duty);

void
rect_gen_deinit(rect_gen_t *gen);

#endif
