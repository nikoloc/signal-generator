#ifndef DAC_DMA_GEN_H
#define DAC_DMA_GEN_H

#include "driver/dac_continuous.h"
#include "gen.h"
#include "util/ints.h"

// TODO: check if we can improve the precision here by changing this parametar, of something else deeper in the esp
// implementation configuration.
#define DAC_DMA_GEN_BUFFER_SIZE 512

// this is a generic generator implementation that uses dac in dma continuous mode to generate a signal from the
// `dac_dma_gen_t.buffer`. users should set the `dac_dma_gen_t->generate_points` function thats going to fill the buffer
// with the desired data points.
//
// see the reference usage for the triangle signal generation

typedef struct dac_dma_gen_interface {
    // should return bitmask of errors if the params are not good
    u32 (*verify_params)(gen_params_t *params);
    u32 (*generate_points)(u8 *buffer, u32 count, gen_params_t *params);
} dac_dma_gen_interface_t;

typedef struct dac_dma_gen {
    gen_t base_gen;

    const dac_dma_gen_interface_t *impl;

    u8 buffer[DAC_DMA_GEN_BUFFER_SIZE];

    dac_continuous_handle_t dac_handle;
} dac_dma_gen_t;

void
dac_dma_gen_init(dac_dma_gen_t *gen, const dac_dma_gen_interface_t *impl);

#endif
