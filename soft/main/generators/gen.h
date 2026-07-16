#ifndef GEN_H
#define GEN_H

#include "esp_err.h"
#include "util/ints.h"

// this file containes a generic genartor interface. anything implementing this interface is considered a generator. see
// `dac_dma_gen.h`, `triangle_gen.h` `rect_gen.h` and `sine_gen.h` and appropriate sources for references

typedef struct gen_params {
    int freq;
    float offset;
    float symmetry;
} gen_params_t;

struct gen;

typedef enum gen_params_error {
    GEN_ERROR_NONE = 0,
    GEN_ERROR_FREQ = (1 << 0),
    GEN_ERROR_OFFSET = (1 << 1),
    GEN_ERROR_SYMMETRY = (1 << 2),
    GEN_ERROR_UNKNOWN = (1 << 3),
} gen_params_error_t;

// generator interface, i.e. a struct of function pointers.
typedef struct gen_interface {
    // returns the bitmask of all the parametars that are invalid for this generator. if it returns `GEN_ERROR_NONE`
    // then the subsequent call to `start()` would only fail due to hardware failure.
    u32 (*probe)(struct gen *gen, gen_params_t *params);
    u32 (*start)(struct gen *gen, gen_params_t *params);
    esp_err_t (*stop)(struct gen *gen);
} gen_interface_t;

typedef struct gen {
    const gen_interface_t *impl;
} gen_t;

void
gen_init(gen_t *gen, const gen_interface_t *impl);

u32
gen_probe(gen_t *gen, gen_params_t *params);

u32
gen_start(gen_t *gen, gen_params_t *params);

esp_err_t
gen_stop(gen_t *gen);

#endif
