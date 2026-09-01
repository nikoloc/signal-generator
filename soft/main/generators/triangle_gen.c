#include "triangle_gen.h"

#include "util/constants.h"

static esp_err_t
generate_points(u8 *buffer, u32 count, int freq, float symmetry) {
    if(freq < MIN_TRI_FREQ || freq > MAX_TRI_FREQ || symmetry < 0 || symmetry > 1) {
        return ESP_ERR_INVALID_ARG;
    }

    int peak_index = count * symmetry;

    for(int i = 0; i < peak_index; i++) {
        buffer[i] = (255 * i) / peak_index;
    }

    int falling_steps = count - peak_index;
    for(int i = peak_index; i < count; i++) {
        int steps_from_end = count - 1 - i;
        buffer[i] = (255 * steps_from_end) / falling_steps;
    }

    return ESP_OK;
}

void
triangle_gen_init(dac_dma_gen_t *gen) {
    dac_dma_gen_init(gen, generate_points);
}
