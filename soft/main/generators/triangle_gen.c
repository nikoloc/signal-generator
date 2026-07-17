#include "triangle_gen.h"

#include "util/constants.h"
#include "util/macros.h"

static esp_err_t
generate_points(u8 *buffer, u32 count, int freq, float symmetry) {
    ASSERT(freq >= MIN_TRI_FREQ && freq <= MAX_TRI_FREQ);
    ASSERT(symmetry >= 0 && symmetry <= 1);

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
