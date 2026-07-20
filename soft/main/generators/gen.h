#ifndef GEN_H
#define GEN_H

#include "esp_err.h"

// this file containes a generic genartor interface. anything implementing this interface is considered a generator. see
// `dac_dma_gen.h`, `triangle_gen.h` `rect_gen.h` and `sine_gen.h` and appropriate sources for references

struct gen;

// generator interface, i.e. a struct of function pointers.
typedef struct gen_interface {
    esp_err_t (*start)(struct gen *gen, int freq, float symmetry);
    esp_err_t (*stop)(struct gen *gen);
} gen_interface_t;

typedef struct gen {
    const gen_interface_t *impl;
} gen_t;

void
gen_init(gen_t *gen, const gen_interface_t *impl);

esp_err_t
gen_start(gen_t *gen, int freq, float symmetry);

esp_err_t
gen_stop(gen_t *gen);

#endif
