#include "offset.h"

#include <math.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "main.h"
#include "util/constants.h"
#include "util/macros.h"

void
offset_init(void) {
    // ground all the pins by default
    gpio_reset_pin(O_OFFSET_POSITIVE);
    gpio_set_direction(O_OFFSET_POSITIVE, GPIO_MODE_OUTPUT);
    gpio_set_level(O_OFFSET_POSITIVE, 0);

    gpio_reset_pin(O_OFFSET_NEGATIVE);
    gpio_set_direction(O_OFFSET_NEGATIVE, GPIO_MODE_OUTPUT);
    gpio_set_level(O_OFFSET_NEGATIVE, 0);
}

esp_err_t
offset_enable(float value) {
    ASSERT(IN_RANGE(value, MIN_OFFSET, MAX_OFFSET));

    // from the voltage value in `value` we need to do few things:
    //
    // 1) divide the value by 4, since the amplifier has a gain of 4
    //
    // 2) figure out the sign of the offset and enable the appropriate output pwm signal to either the non inverting or
    // the inverting amplifier
    //
    // 3) calculate the duty cycle of the pwm signal in the range 0-255. since the max voltage
    // coresponds to 3.3V, we calculate so as
    //      duty = value * 255 / 3.3

    value /= 4;

    bool is_negative = value < 0;
    int duty = fabsf(value) * 255.0f / VDD;

    int pin = is_negative ? O_OFFSET_NEGATIVE : O_OFFSET_POSITIVE;

    ledc_timer_config_t timer_conf = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            // use timer 1 for this, since the timer 0 is used for the rect generator
            .timer_num = LEDC_TIMER_1,
            .duty_resolution = LEDC_TIMER_8_BIT,
            // NOTE: we use 100kHz for the pwm freq, this number should be tested and adjusted as needed
            .freq_hz = 100E3,
            .clk_cfg = LEDC_AUTO_CLK,
    };

    esp_err_t err = ledc_timer_config(&timer_conf);
    if(err) {
        return err;
    }

    ledc_channel_config_t channel_conf = {
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            // NOTE: same as above
            .channel = LEDC_CHANNEL_1,
            .timer_sel = LEDC_TIMER_1,
            .gpio_num = pin,
            .duty = duty,
            .hpoint = 0,
    };

    err = ledc_channel_config(&channel_conf);
    if(err) {
        return err;
    }

    return ESP_OK;
}

void
offset_disable(void) {
    // remove the pin from the ledc system altogether
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);

    // these cant fail
    gpio_reset_pin(O_OFFSET_POSITIVE);
    gpio_set_direction(O_OFFSET_POSITIVE, GPIO_MODE_OUTPUT);
    gpio_set_level(O_OFFSET_POSITIVE, 0);

    gpio_reset_pin(O_OFFSET_NEGATIVE);
    gpio_set_direction(O_OFFSET_NEGATIVE, GPIO_MODE_OUTPUT);
    gpio_set_level(O_OFFSET_NEGATIVE, 0);
}
