#include "ctl.h"

#include "esp_log.h"
#include "generators/rect_gen.h"
#include "generators/sine_gen.h"
#include "generators/triangle_gen.h"
#include "util/macros.h"

static const char *TAG = "CTL";

const char *ctl_signal_type_to_string[] = {
        [CTL_SIGNAL_TYPE_SINE] = "sine",
        [CTL_SIGNAL_TYPE_RECT] = "rect",
        [CTL_SIGNAL_TYPE_TRIANGLE] = "tri",
};

static struct g {
    ctl_signal_type_t type;

    // map of signal types to appropriate generators bases, see `ctl_init()`
    gen_t *signal_type_to_gen[_CTL_SIGNAL_TYPE_COUNT];

    sine_gen_t sine_gen;
    rect_gen_t rect_gen;
    dac_dma_gen_t triangle_gen;
} g;

void
ctl_init(void) {
    sine_gen_init(&g.sine_gen);
    rect_gen_init(&g.rect_gen);
    triangle_gen_init(&g.triangle_gen);

    g.signal_type_to_gen[CTL_SIGNAL_TYPE_SINE] = &g.sine_gen.base_gen;
    g.signal_type_to_gen[CTL_SIGNAL_TYPE_RECT] = &g.rect_gen.base_gen;
    g.signal_type_to_gen[CTL_SIGNAL_TYPE_TRIANGLE] = &g.triangle_gen.base_gen;
}

esp_err_t
ctl_enable(ctl_signal_type_t type, ctl_params_t *params) {
    ASSERT(g.type != CTL_SIGNAL_TYPE_NONE);

    gen_t *gen = g.signal_type_to_gen[type];

    ESP_LOGI(TAG, "starting %s generator with parametars - freq: %d, symmetry: %.2f, offset: %.2f",
            ctl_signal_type_to_string[type], params->freq, params->symmetry, params->offset);

    esp_err_t err = gen_start(gen, params->freq, params->symmetry);
    if(err) {
        ESP_LOGE(TAG, "generator error: %d", err);
        return err;
    }

    g.type = type;
    ESP_LOGI(TAG, "started generator successfully");

    return ESP_OK;
}

esp_err_t
ctl_disable(void) {
    gen_t *current = g.signal_type_to_gen[g.type];

    esp_err_t err = gen_stop(current);
    if(err) {
        return err;
    }

    ESP_LOGI(TAG, "generator stopped");

    g.type = CTL_SIGNAL_TYPE_NONE;

    return ESP_OK;
}

bool
ctl_is_enabled(void) {
    return g.type != CTL_SIGNAL_TYPE_NONE;
}
