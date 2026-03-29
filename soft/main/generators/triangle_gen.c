#include "triangle_gen.h"

static esp_err_t
generate_points(u8 *buffer, gen_params_t *params) {
    if(params->freq < 10 || params->symmetry < 0 || params->symmetry > 1) {
        return ESP_ERR_INVALID_ARG;
    }

    int peak_index = DAC_DMA_GEN_BUFFER_SIZE * params->symmetry;

    for(int i = 0; i < peak_index; i++) {
        buffer[i] = (255 * i) / peak_index;
    }

    int falling_steps = DAC_DMA_GEN_BUFFER_SIZE - peak_index;
    for(int i = peak_index; i < DAC_DMA_GEN_BUFFER_SIZE; i++) {
        int steps_from_end = DAC_DMA_GEN_BUFFER_SIZE - 1 - i;
        buffer[i] = (255 * steps_from_end) / falling_steps;
    }

    return ESP_OK;
}

void
triangle_gen_init(dac_dma_gen_t *gen) {
    dac_dma_gen_init(gen, generate_points);
}
