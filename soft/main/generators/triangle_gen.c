#include "triangle_gen.h"

#include "util/constants.h"

static u32
verify_params(gen_params_t *params) {
    u32 ret = GEN_ERROR_NONE;

    if(params->freq < MIN_TRI_FREQ || params->freq > MAX_TRI_FREQ) {
        ret |= GEN_ERROR_FREQ;
    }

    if(params->symmetry < 0 || params->symmetry > 1) {
        ret |= GEN_ERROR_SYMMETRY;
    }

    if(params->offset < MIN_OFFSET || params->offset > MAX_OFFSET) {
        ret |= GEN_ERROR_OFFSET;
    }

    return ret;
}

static u32
generate_points(u8 *buffer, u32 count, gen_params_t *params) {
    u32 err = verify_params(params);
    if(err) {
        return err;
    }

    int peak_index = count * params->symmetry;

    for(int i = 0; i < peak_index; i++) {
        buffer[i] = (255 * i) / peak_index;
    }

    int falling_steps = count - peak_index;
    for(int i = peak_index; i < count; i++) {
        int steps_from_end = count - 1 - i;
        buffer[i] = (255 * steps_from_end) / falling_steps;
    }

    return GEN_ERROR_NONE;
}

static const dac_dma_gen_interface_t triangle_gen_impl = {
        .verify_params = verify_params,
        .generate_points = generate_points,
};

void
triangle_gen_init(dac_dma_gen_t *gen) {
    dac_dma_gen_init(gen, &triangle_gen_impl);
}
