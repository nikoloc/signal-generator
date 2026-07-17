#include "ui.h"

#include <inttypes.h>
#include <math.h>

#include "ctl.h"
#include "esp_log.h"
#include "lcd.h"
#include "util/constants.h"

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
        // in order to display a zero properly
        bool is_zero;
    } input;

    ctl_signal_type_t type;
    gen_params_t params;
    gen_params_error_t err;
} g;

static void
go_home(void) {
    // set everything to default
    g.menu = UI_MENU_HOME;
    g.input.whole = 0;
    g.input.frac = 0;
    g.input.frac_count = 0;
    g.input.dot = false;
    g.input.is_zero = false;
    g.input.sign = 1;
}

void
ui_init(void) {
    go_home();
}

static void
handle_digit(int d) {
    if(g.menu == UI_MENU_HOME) {
        if(d == 1) {
            g.menu = UI_MENU_TYPE;
        } else if(d == 2) {
            g.menu = UI_MENU_FREQ;
        } else if(d == 3) {
            g.menu = UI_MENU_OFFSET;
        } else if(d == 4 && g.type != CTL_SIGNAL_TYPE_SINE) {
            // sine does not have symmetry, so pass in that case
            g.menu = UI_MENU_SYMMETRY;
        }
    } else if(g.menu == UI_MENU_TYPE && d < _CTL_SIGNAL_TYPE_COUNT) {
        g.type = d;

        // we can decide if we want all settings to reset upon signal type change
        g.params.freq = 0;
        g.params.offset = 0;
        g.params.symmetry = 0;

        go_home();
    } else if(g.menu == UI_MENU_FREQ || g.menu == UI_MENU_OFFSET || g.menu == UI_MENU_SYMMETRY) {
        // handle input for this type
        if(g.input.dot) {
            g.input.frac_count++;
            g.input.frac = g.input.frac * 10 + d;
        } else {
            g.input.whole = g.input.whole * 10 + d;
        }

        // TODO: this may work, but for the wrong reason, currently it is going to set it to true, for any zero
        // anywhere. so it works for the cause we need but it does not actually keep the info if the number is zero, as
        // the name would suggest. investigate.
        if(d == 0) {
            g.input.is_zero = true;
        }
    }
}

static inline float
floating_input(void) {
    return g.input.sign * (g.input.whole + g.input.frac / pow(10, g.input.frac_count));
}

static void
print_decimal(void) {
    float value = floating_input();
    lcd_tprintf(1, 0, ">%c", (g.input.sign < 0) ? '-' : ' ');

    if(value || g.input.is_zero) {
        if(g.input.frac_count == 0) {
            lcd_tprintf(1, 2, "%d%s", (value < 0) ? -value : value, (g.input.dot) ? "." : "");
        } else if(g.input.frac_count == 1) {
            lcd_tprintf(1, 2, "%.1f", (value < 0) ? -value : value);
        } else {
            lcd_tprintf(1, 2, "%.2f", (value < 0) ? -value : value);
        }
    }
}

void
ui_render(void) {
    lcd_tclear();

    switch(g.menu) {
        case UI_MENU_HOME: {
            lcd_tprintf(0, 0, "1.type:%s", ctl_signal_type_to_string[g.type]);
            lcd_tprintf(1, 0, "2.freq:%d", g.params.freq);
            lcd_tprintf(2, 0, "3.offs:%.2f", g.params.offset);
            if(g.type != CTL_SIGNAL_TYPE_SINE) {
                lcd_tprintf(3, 0, "4.sim:%.2f", g.params.symmetry);
            }

            break;
        }
        case UI_MENU_TYPE: {
            lcd_tprintf(0, 0, "select type:");

            for(size_t i = 1; i < _CTL_SIGNAL_TYPE_COUNT; i++) {
                lcd_tprintf(i, 0, "%d. %s", i, ctl_signal_type_to_string[i]);
            }

            break;
        }
        case UI_MENU_FREQ: {
            lcd_tprintf(0, 0, "type frequency");
            lcd_tprintf(1, 0, "> ", g.input.whole);
            if(g.input.whole) {
                lcd_tprintf(1, 2, "%d", g.input.whole);
            }

            lcd_tprintf(3, 0, "%d-%d", signal_type_to_freq_range[g.type][0], signal_type_to_freq_range[g.type][1]);

            break;
        }

        case UI_MENU_OFFSET: {
            lcd_tprintf(0, 0, "type offset");
            print_decimal();
            lcd_tprintf(3, 0, "%d-%d", MIN_OFFSET, MAX_OFFSET);

            break;
        }
        case UI_MENU_SYMMETRY: {
            lcd_tprintf(0, 0, "type symmetry");
            print_decimal();
            lcd_tprintf(3, 0, "%d-%d", 0, 1);

            break;
        }
    }
}

void
ui_handle_key(key_t key) {
    ESP_LOGI(TAG, "key pressed: %d\n", key);

    switch(key) {
        case KEY_ENABLE: {
            if(ctl_is_enabled()) {
                ctl_disable();
            } else {
                g.err = ctl_enable(g.type, &g.params);
                if(g.err) {
                    // set the asterixes here
                }

                go_home();
            }

            break;
        }
        case KEY_OK: {
            if(g.menu == UI_MENU_FREQ) {
                // TODO: BUG: this currently does not work as we did not update the `g.params`. after the refactor it is
                // not going to be implemented this way so i am just not going to bother now
                if(ctl_probe(g.type, &g.params) & GEN_ERROR_FREQ) {
                    // do not accept this input
                    // TODO: handle the error somehow here, maybe add ! next to the range or something
                } else {
                    g.params.freq = g.input.whole;
                    go_home();
                }
            } else if(g.menu == UI_MENU_OFFSET) {
                // double check this
                if(ctl_probe(g.type, &g.params) & GEN_ERROR_OFFSET) {
                    // do not accept this input
                    // TODO: handle the error somehow here, maybe add ! next to the range or something
                } else {
                    g.params.offset = floating_input();
                    go_home();
                }
            } else if(g.menu == UI_MENU_SYMMETRY) {
                if(ctl_probe(g.type, &g.params) & GEN_ERROR_SYMMETRY) {
                    // do not accept this input
                    // TODO: handle the error somehow here, maybe add ! next to the range or something
                } else {
                    g.params.symmetry = floating_input();
                    go_home();
                }
            }

            break;
        }
        case KEY_CANCEL: {
            go_home();

            break;
        }
        case KEY_BACKSLASH: {
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
