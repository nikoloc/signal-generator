#include "main.h"

#include "driver/gpio.h"
#include "generators/dac_dma_gen.h"
#include "generators/gen.h"
#include "generators/rect_gen.h"
#include "generators/sine_gen.h"
#include "generators/triangle_gen.h"
#include "isr_mgr.h"

typedef enum signal_type {
    SIGNAL_TYPE_SINE = 0,
    SIGNAL_TYPE_RECT,
    SIGNAL_TYPE_TRIANGLE,
    _SIGNAL_TYPE_COUNT,
} signal_type_t;

static const int pin_to_signal_type[] = {
        [I_SELECT_SINE] = SIGNAL_TYPE_SINE,
        [I_SELECT_RECT] = SIGNAL_TYPE_RECT,
        [I_SELECT_TRIANGLE] = SIGNAL_TYPE_TRIANGLE,
};

static const char *signal_type_to_string[] = {
        [SIGNAL_TYPE_SINE] = "sine",
        [SIGNAL_TYPE_RECT] = "rect",
        [SIGNAL_TYPE_TRIANGLE] = "triangle",
};

static struct g {
    bool enabled;
    gen_params_t params;

    signal_type_t current_signal_type;

    // map of signal types to appropriate generators bases, see `init_gens()`
    gen_t *signal_type_to_gen[_SIGNAL_TYPE_COUNT];

    sine_gen_t sine_gen;
    rect_gen_t rect_gen;
    dac_dma_gen_t triangle_gen;
} g;

static void
switch_signal_type(signal_type_t type) {
    if(g.enabled) {
        // we dont switch signals while active
        return;
    }

    g.current_signal_type = type;
    printf("current mode: %s\n", signal_type_to_string[type]);
}

static void
show_gen_error(esp_err_t err) {
    if(err == ESP_ERR_INVALID_ARG) {
        printf("bad parametars\n");
    } else if(err) {
        printf("something went wrong\n");
    }
}

static void
enable(void) {
    // reset the pin just to be sure
    gpio_reset_pin(O_SIGNAL);
    gen_t *current = g.signal_type_to_gen[g.current_signal_type];

    esp_err_t err = gen_start(current, &g.params);
    if(err) {
        show_gen_error(err);
        return;
    }

    g.enabled = true;
    printf("enabled\n");
}

static void
disable(void) {
    gen_t *current = g.signal_type_to_gen[g.current_signal_type];

    esp_err_t err = gen_stop(current);
    if(err) {
        show_gen_error(err);
        return;
    }

    g.enabled = false;
    printf("disabled\n");
}

static void
toggle_enabled(void) {
    if(g.enabled) {
        disable();
    } else {
        enable();
    }
}

static void
isr_callback(pin_t *pin) {
    switch(pin->number) {
        case I_ENABLE: {
            toggle_enabled();
            break;
        }
        case I_PLUS: {
            printf("plus\n");
            break;
        }
        case I_MINUS: {
            printf("minus\n");
            break;
        }
        case I_UP: {
            printf("up\n");
            break;
        }
        case I_DOWN: {
            printf("down\n");
            break;
        }
        default: {
            switch_signal_type(pin_to_signal_type[pin->number]);
            break;
        }
    }
}

static void
init_io(void) {
    ESP_ERROR_CHECK(isr_mgr_init(isr_callback));

    // different modes
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_SELECT_SINE, GPIO_INTR_NEGEDGE, PULL_UP, 200));
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_SELECT_RECT, GPIO_INTR_NEGEDGE, PULL_UP, 200));
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_SELECT_TRIANGLE, GPIO_INTR_NEGEDGE, PULL_NONE,
            200));  // does not have an internal pullup/down

    // plus/minus
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_PLUS, GPIO_INTR_NEGEDGE, PULL_UP, 200));
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_MINUS, GPIO_INTR_NEGEDGE, PULL_UP, 200));

    // up/down, dont have internal pullup/down
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_UP, GPIO_INTR_NEGEDGE, PULL_NONE, 200));
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_DOWN, GPIO_INTR_NEGEDGE, PULL_NONE, 200));

    // enable
    ESP_ERROR_CHECK(isr_mgr_add_pin(I_ENABLE, GPIO_INTR_NEGEDGE, PULL_UP, 200));
}

static void
init_gens(void) {
    sine_gen_init(&g.sine_gen);
    rect_gen_init(&g.rect_gen);
    triangle_gen_init(&g.triangle_gen);

    g.signal_type_to_gen[SIGNAL_TYPE_SINE] = &g.sine_gen.base_gen;
    g.signal_type_to_gen[SIGNAL_TYPE_RECT] = &g.rect_gen.base_gen;
    g.signal_type_to_gen[SIGNAL_TYPE_TRIANGLE] = &g.triangle_gen.base_gen;
}

void
app_main(void) {
    printf("initilizing the generator...\n");

    printf("initilizing the io...\n");
    init_io();

    printf("initilizing the generators...\n");
    init_gens();

    printf("initilizition successful!\n");

    // hard code the parametars for testing
    // TODO: check if 0 values for duty/symmetry work
    g.params = (gen_params_t){
            .freq = 10000,
            .duty = 0.5,
            .symmetry = 0.5,
    };

    while(1) {
        vTaskDelay(1000);
    }
}
