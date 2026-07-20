#include "ctl.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "generators/none_gen.h"
#include "generators/rect_gen.h"
#include "generators/sine_gen.h"
#include "generators/triangle_gen.h"
#include "main.h"
#include "util/macros.h"

static const char *TAG = "CTL";

const char *ctl_signal_type_to_string[] = {
        [CTL_SIGNAL_TYPE_NONE] = "none",
        [CTL_SIGNAL_TYPE_SINE] = "sine",
        [CTL_SIGNAL_TYPE_RECT] = "rect",
        [CTL_SIGNAL_TYPE_TRIANGLE] = "tri",
};

static struct g {
    bool is_enabled;
    ctl_signal_type_t type;

    // map of signal types to appropriate generators bases, see `ctl_init()`
    gen_t *signal_type_to_gen[_CTL_SIGNAL_TYPE_COUNT];

    none_gen_t none_gen;
    sine_gen_t sine_gen;
    rect_gen_t rect_gen;
    dac_dma_gen_t triangle_gen;
} g;

void
ctl_init(void) {
    none_gen_init(&g.none_gen);
    sine_gen_init(&g.sine_gen);
    rect_gen_init(&g.rect_gen);
    triangle_gen_init(&g.triangle_gen);

    g.signal_type_to_gen[CTL_SIGNAL_TYPE_NONE] = &g.none_gen.base_gen;
    g.signal_type_to_gen[CTL_SIGNAL_TYPE_SINE] = &g.sine_gen.base_gen;
    g.signal_type_to_gen[CTL_SIGNAL_TYPE_RECT] = &g.rect_gen.base_gen;
    g.signal_type_to_gen[CTL_SIGNAL_TYPE_TRIANGLE] = &g.triangle_gen.base_gen;

    // configure the status led
    gpio_config_t conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << O_STATUS_LED,
    };

    ESP_ERROR_CHECK(gpio_config(&conf));
    ESP_ERROR_CHECK(gpio_set_level(O_SIGNAL, 0));
}

esp_err_t
ctl_enable(ctl_signal_type_t type, ctl_params_t *params) {
    if(g.is_enabled) {
        return ESP_OK;
    }

    gen_t *gen = g.signal_type_to_gen[type];

    ESP_LOGI(TAG, "starting %s generator with parametars - freq: %d, symmetry: %.2f, offset: %.2f",
            ctl_signal_type_to_string[type], params->freq, params->symmetry, params->offset);

    esp_err_t err = gen_start(gen, params->freq, params->symmetry);
    if(err) {
        ESP_LOGE(TAG, "generator error: %d", err);
        return err;
    }

    g.is_enabled = true;
    g.type = type;

    // enabled the status led; dont care if fails
    gpio_set_level(O_SIGNAL, 1);

    ESP_LOGI(TAG, "generator started successfully");
    return ESP_OK;
}

esp_err_t
ctl_disable(void) {
    if(!g.is_enabled) {
        return ESP_OK;
    };

    gen_t *current = g.signal_type_to_gen[g.type];

    esp_err_t err = gen_stop(current);
    if(err) {
        ESP_LOGE(TAG, "generator error: %d", err);
        return err;
    }

    g.is_enabled = false;

    gpio_set_level(O_SIGNAL, 0);

    ESP_LOGI(TAG, "generator stopped successfully");
    return ESP_OK;
}

bool
ctl_is_enabled(void) {
    return g.is_enabled;
}
