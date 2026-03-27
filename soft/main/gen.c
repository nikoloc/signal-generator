#include "gen.h"

#include "driver/gpio.h"
#include "isr_mgr.h"
#include "rect_gen.h"
#include "sine_gen.h"

#define I_SELECT_SINE 14
#define I_SELECT_RECT 12

#define I_ENABLE 27

typedef enum gen_mode {
    GEN_MODE_SINE = 0,
    GEN_MODE_RECT,
} gen_mode_t;

const static int pin_to_mode[] = {
        [I_SELECT_SINE] = GEN_MODE_SINE,
        [I_SELECT_RECT] = GEN_MODE_RECT,
};

const static char *mode_to_string[] = {
        [GEN_MODE_SINE] = "sine",
        [GEN_MODE_RECT] = "rect",
};

static struct g {
    bool enabled;
    gen_mode_t current_mode;

    sine_gen_t sine;
    rect_gen_t rect;

    u32 freq;
    u8 ampl;
    u8 duty;
} g;

static void
switch_mode(gen_mode_t mode) {
    if(g.enabled) {
        // we dont switch mode while its active
        return;
    }

    g.current_mode = mode;
    printf("current mode: %s\n", mode_to_string[mode]);
}

static void
check_for_gen_error(esp_err_t err) {
    switch(err) {
        case ESP_OK: {
            return;
        }
        case ESP_ERR_INVALID_ARG: {
            printf("losi parametri\n");
            return;
        }
        default: {
            printf("nesto ne valja\n");
            return;
        }
    }
}

static void
enable_current_mode(void) {
    // reset the pin just to be sure
    gpio_reset_pin(25);

    switch(g.current_mode) {
        case GEN_MODE_SINE: {
            esp_err_t err = sine_gen_init(&g.sine, g.freq, g.ampl);
            check_for_gen_error(err);
            break;
        }
        case GEN_MODE_RECT: {
            esp_err_t err = rect_gen_init(&g.rect, g.freq, g.ampl, g.duty);
            check_for_gen_error(err);
            break;
        }
        default: {
            unreachable();
        }
    }
}

static void
disable_current_mode(void) {
    switch(g.current_mode) {
        case GEN_MODE_SINE: {
            sine_gen_deinit(&g.sine);
            break;
        }
        case GEN_MODE_RECT: {
            rect_gen_deinit(&g.rect);
            break;
        }
        default: {
            unreachable();
        }
    }

    // ground the pin so there is no residue output
    gpio_reset_pin(25);
    gpio_set_direction(25, GPIO_MODE_OUTPUT);
    gpio_set_level(25, 0);
}

static void
toggle_enabled(void) {
    g.enabled = !g.enabled;

    if(g.enabled) {
        printf("enabled\n");
        enable_current_mode();
    } else {
        printf("disabled\n");
        disable_current_mode();
    }
}

static void
isr_callback(int pin) {
    if(pin == I_ENABLE) {
        toggle_enabled();
    } else {
        switch_mode(pin_to_mode[pin]);
    }
}

void
gen_setup_and_run(void) {
    ESP_ERROR_CHECK(isr_mgr_init(isr_callback));
    // add one button for sine wave
    ESP_ERROR_CHECK(isr_mgr_add_source(I_SELECT_SINE, GPIO_INTR_NEGEDGE, PULL_UP));
    // for rect wave
    ESP_ERROR_CHECK(isr_mgr_add_source(I_SELECT_RECT, GPIO_INTR_NEGEDGE, PULL_UP));
    // for enabling the output
    ESP_ERROR_CHECK(isr_mgr_add_source(I_ENABLE, GPIO_INTR_NEGEDGE, PULL_UP));

    // hard code the parametars for testing
    g.freq = 2000;
    g.ampl = 127;
    g.duty = 200;

    while(1) {
        vTaskDelay(100);
    }
}
