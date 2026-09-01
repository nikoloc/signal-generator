#include "ui.h"

#include <inttypes.h>
#include <math.h>

#include "ctl.h"
#include "esp_log.h"
#include "lcd.h"
#include "util/constants.h"
#include "util/macros.h"

static const char *TAG = "UI";

const int signal_type_to_freq_range[][2] = {
        [CTL_SIGNAL_TYPE_SINE] = {MIN_SINE_FREQ, MAX_SINE_FREQ},
        [CTL_SIGNAL_TYPE_RECT] = {MIN_RECT_FREQ, MAX_RECT_FREQ},
        [CTL_SIGNAL_TYPE_TRIANGLE] = {MIN_TRI_FREQ, MAX_TRI_FREQ},
};

typedef enum ui_menu {
    UI_MENU_HOME = 0,
    UI_MENU_TYPE,
    UI_MENU_FREQ,
    UI_MENU_OFFSET,
    UI_MENU_SYMMETRY,
} ui_menu_t;

static struct g {
    ui_menu_t menu;

    struct {
        int sign;
        int whole, frac;
        int frac_count;
        bool dot;
    } input;

    // generator params
    ctl_signal_type_t type;
    ctl_params_t params;
} g;

static void
go_home(void) {
    // set everything to default
    g.menu = UI_MENU_HOME;
    g.input.whole = 0;
    g.input.frac = 0;
    g.input.frac_count = 0;
    g.input.dot = false;
    g.input.sign = 1;
}

void
ui_init(void) {
    g.type = CTL_SIGNAL_TYPE_NONE;
    go_home();

    ui_render();
}

static void
handle_digit_home(int d) {
    switch(d) {
        case 1: {
            g.menu = UI_MENU_TYPE;
            break;
        }
        case 2: {
            if(g.type != CTL_SIGNAL_TYPE_NONE) {
                g.menu = UI_MENU_FREQ;
            }
            break;
        }

        case 3: {
            if(g.type != CTL_SIGNAL_TYPE_NONE) {
                g.menu = UI_MENU_OFFSET;
            }
            break;
        }
        case 4: {
            if(g.type == CTL_SIGNAL_TYPE_RECT || g.type == CTL_SIGNAL_TYPE_TRIANGLE) {
                // only these have symmetry
                g.menu = UI_MENU_SYMMETRY;
            }
            break;
        }
    }
}

static void
handle_digit_type(int d) {
    if(d >= _CTL_SIGNAL_TYPE_COUNT) {
        return;
    }

    g.type = d;

    // reset params on type change
    g.params.freq = 0;
    g.params.offset = 0;
    g.params.symmetry = 0;

    go_home();
}

static void
handle_digit_input(int d) {
    if(g.input.dot) {
        g.input.frac_count++;
        g.input.frac = g.input.frac * 10 + d;
    } else {
        g.input.whole = g.input.whole * 10 + d;
    }
}

static void
handle_digit(int d) {
    switch(g.menu) {
        case UI_MENU_HOME: {
            handle_digit_home(d);
            break;
        }
        case UI_MENU_TYPE: {
            handle_digit_type(d);
            break;
        }
        case UI_MENU_FREQ:
        case UI_MENU_OFFSET:
        case UI_MENU_SYMMETRY: {
            handle_digit_input(d);
            break;
        }
    }
}

static inline float
float_input(void) {
    return g.input.sign * (g.input.whole + g.input.frac / pow(10, g.input.frac_count));
}

static inline float
int_input(void) {
    return g.input.sign * g.input.whole;
}

static void
print_decimal(void) {
    char sign = g.input.sign < 0 ? '-' : ' ';

    if(g.input.frac == 0) {
        if(g.input.dot) {
            lcd_printf(1, 0, ">%c%d.", sign, g.input.whole);
        } else {
            lcd_printf(1, 0, ">%c%d", sign, g.input.whole);
        }
    } else {
        lcd_printf(1, 0, ">%c%d.%d", sign, g.input.whole, g.input.frac);
    }
    // if(g.input.is_zero) {
    //     return;
    // }
    //
    // if(g.input.whole == 0 && g.input.frac == 0) {
    //     lcd_printf(1, 0, ">%c%s", (g.input.sign < 0) ? '-' : ' ');
    //     return;
    // }
    //
    // lcd_printf(1, 0, ">%c", (g.input.sign < 0) ? '-' : ' ');
    //
    // if(g.input.frac_count == 0) {
    //     lcd_printf(1, 2, "%d%s", g.input.whole, (g.input.dot) ? "." : "");
    // } else {
    //     float value = g.input.whole + g.input.frac / pow(10, g.input.frac_count);
    //     if(g.input.frac_count == 1) {
    //         lcd_printf(1, 2, "%.1f", value);
    //     } else {
    //         lcd_printf(1, 2, "%.2f", value);
    //     }
    // }
}

void
ui_render(void) {
    lcd_clear();

    switch(g.menu) {
        case UI_MENU_HOME: {
            lcd_printf(0, 0, "1.type:%s", ctl_signal_type_to_string[g.type]);

            if(g.type != CTL_SIGNAL_TYPE_NONE) {
                lcd_printf(1, 0, "2.freq:%d", g.params.freq);
            }
            if(g.type != CTL_SIGNAL_TYPE_NONE) {
                lcd_printf(2, 0, "3.offs:%.2f", g.params.offset);
            }
            if(g.type == CTL_SIGNAL_TYPE_RECT || g.type == CTL_SIGNAL_TYPE_TRIANGLE) {
                lcd_printf(3, 0, "4.sim:%.2f", g.params.symmetry);
            }

            break;
        }
        case UI_MENU_TYPE: {
            for(size_t i = 0; i < _CTL_SIGNAL_TYPE_COUNT; i++) {
                lcd_printf(i, 0, "%d. %s", i, ctl_signal_type_to_string[i]);
            }

            break;
        }
        case UI_MENU_FREQ: {
            lcd_printf(0, 0, "type frequency");
            print_decimal();
            lcd_printf(3, 0, "%d..%d", signal_type_to_freq_range[g.type][0], signal_type_to_freq_range[g.type][1]);

            break;
        }

        case UI_MENU_OFFSET: {
            lcd_printf(0, 0, "type offset");
            print_decimal();
            lcd_printf(3, 0, "%d..%d", MIN_OFFSET, MAX_OFFSET);

            break;
        }
        case UI_MENU_SYMMETRY: {
            lcd_printf(0, 0, "type symmetry");
            print_decimal();
            lcd_printf(3, 0, "%d..%d", 0, 1);

            break;
        }
    }
}

static void
handle_ok(void) {
    switch(g.menu) {
        case UI_MENU_HOME: {
            // noop
            break;
        }
        case UI_MENU_TYPE: {
            // noop
            break;
        }
        case UI_MENU_FREQ: {
            int value = int_input();
            if(IN_RANGE(value, signal_type_to_freq_range[g.type][0], signal_type_to_freq_range[g.type][1])) {
                g.params.freq = value;
                go_home();
            }
            break;
        }
        case UI_MENU_OFFSET: {
            float value = float_input();
            if(IN_RANGE(value, MIN_OFFSET, MAX_OFFSET)) {
                g.params.offset = value;
                go_home();
            }
            break;
        }
        case UI_MENU_SYMMETRY: {
            float value = float_input();
            if(IN_RANGE(value, 0, 1)) {
                g.params.symmetry = value;
                go_home();
            }
            break;
        }
    }
}

static void
handle_enable(void) {
    if(ctl_is_enabled()) {
        ctl_disable();
    } else {
        ctl_enable(g.type, &g.params);
        go_home();
    }
}

static void
handle_backslash(void) {
    if(g.menu == UI_MENU_FREQ || g.menu == UI_MENU_OFFSET || g.menu == UI_MENU_SYMMETRY) {
        if(g.input.dot) {
            if(g.input.frac_count > 0) {
                g.input.frac /= 10;
                g.input.frac_count--;
            } else {
                g.input.dot = false;
            }
        } else {
            g.input.whole /= 10;
        }
    }
}

void
ui_handle_key(key_t key) {
    ESP_LOGI(TAG, "key pressed: %d", key);

    // do not let the user do anything if the generator is enabled
    if(ctl_is_enabled() && key != KEY_ENABLE) {
        return;
    }

    switch(key) {
        case KEY_ENABLE: {
            handle_enable();
            break;
        }
        case KEY_OK: {
            handle_ok();
            break;
        }
        case KEY_CANCEL: {
            go_home();
            break;
        }
        case KEY_BACKSLASH: {
            handle_backslash();
            break;
        }
        case KEY_SIGN: {
            if(g.menu == UI_MENU_FREQ || g.menu == UI_MENU_OFFSET || g.menu == UI_MENU_SYMMETRY) {
                g.input.sign *= -1;
            }

            break;
        }
        case KEY_DOT: {
            if(g.menu == UI_MENU_FREQ || g.menu == UI_MENU_OFFSET || g.menu == UI_MENU_SYMMETRY) {
                g.input.dot = true;
            }

            break;
        }
        default: {
            // only digits are left
            handle_digit(key);
            break;
        }
    }

    // rerender the scene
    ui_render();
}
