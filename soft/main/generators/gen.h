#ifndef GEN_H
#define GEN_H

#include "esp_err.h"
#include "util/ints.h"

// this file containes a generic genartor interface. anything implementing this interface is considered a generator. see
// `dac_dma_gen.h`, `triangle_gen.h` `rect_gen.h` and `sine_gen.h` and appropriate sources for references

typedef struct gen_params {
    u32 freq;
    float duty;
    float symmetry;
} gen_params_t;

struct gen;

// generator interface, i.e. a struct of function pointers. we might want to add hot reloading in the future, that is,
// the abillity to change the parametars on the fly, which should also go here.
typedef struct gen_interface {
    esp_err_t (*start)(struct gen *gen, gen_params_t *params);
    esp_err_t (*stop)(struct gen *gen);
} gen_interface_t;

typedef struct gen {
    const gen_interface_t *impl;
    bool active;
} gen_t;

void
gen_init(gen_t *gen, const gen_interface_t *impl);

esp_err_t
gen_start(gen_t *gen, gen_params_t *params);

esp_err_t
gen_stop(gen_t *gen);

#endif
