#include "regulator.h"

#include "driver/dac_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/adc_types.h"
#include "soc/soc_caps.h"
#include "util/ints.h"
#include "util/macros.h"

#define REGULATION_CONSTANT (0.05)

#define ADC_ATTEN (ADC_ATTEN_DB_12)
#define ADC_UNIT (ADC_UNIT_1)
#define ADC_CHANNEL (0)
#define ADC_BITWIDTH (12)

#define DAC_UNIT (DAC_CHAN_1)

#define REGULATOR_WINDOW_SIZE (256)

static const char *TAG = "REGULATOR";

static struct {
    bool enabled;

    adc_cali_handle_t cali_handle;
    adc_continuous_handle_t cont_handle;

    dac_oneshot_handle_t dac_handle;

    TaskHandle_t task_handle;

    // in mV
    int target;
    int control;
} g;

static void
calibrate_adc(void) {
    adc_cali_line_fitting_config_t conf = {
            .unit_id = ADC_UNIT,
            .atten = ADC_ATTEN,
            .bitwidth = ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&conf, &g.cali_handle));
}

static void
init_adc(void) {
    calibrate_adc();

    adc_continuous_handle_cfg_t conf = {
            .max_store_buf_size = REGULATOR_WINDOW_SIZE * SOC_ADC_DIGI_DATA_BYTES_PER_CONV,
            .conv_frame_size = REGULATOR_WINDOW_SIZE * SOC_ADC_DIGI_DATA_BYTES_PER_CONV,
            // always want fresh data
            .flags.flush_pool = true,
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&conf, &g.cont_handle));
}

static int
raw_to_voltage(int raw) {
    int voltage;
    // int voltage = raw * 1100 * 4 / 4096;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(g.cali_handle, raw, &voltage));

    return voltage;
}

static int
compar(const void *_a, const void *_b) {
    int a = *(const int *)_a;
    int b = *(const int *)_b;

    return a - b;
}

static int
calc_ampl(adc_continuous_data_t *window, u32 count) {
    // make this static so it does not eat our small stack
    static int arr[REGULATOR_WINDOW_SIZE];
    int size = 0;

    for(u32 i = 0; i < count; i++) {
        if(window[i].valid) {
            arr[size++] = raw_to_voltage(window[i].raw_data);
        }
    }

    if(size == 0) {
        // no data available, bug?
        return 0;
    }

    qsort(arr, size, sizeof(int), compar);

    int ninety_fifth_percent = 0.95f * size;
    return arr[ninety_fifth_percent];
}

static void
update_control(int meas) {
    g.control = CLAMP(g.control + (g.target - meas) * REGULATION_CONSTANT, 1, 3300);

    u8 value = g.control * 255 / 3300;
    // ESP_LOGI(TAG, "setting regulator voltage to %dmV", g.control);
    ESP_ERROR_CHECK(dac_oneshot_output_voltage(g.dac_handle, 150));
}

static void
init_dac(void) {
    dac_oneshot_config_t conf = {
            .chan_id = DAC_UNIT,
    };

    ESP_ERROR_CHECK(dac_oneshot_new_channel(&conf, &g.dac_handle));
}

static void
task(void *_data) {
    // alloacate only once in static memory
    static adc_continuous_data_t data[REGULATOR_WINDOW_SIZE];
    u32 count;

    while(1) {
        if(!g.enabled) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        if(adc_continuous_read_parse(g.cont_handle, data, REGULATOR_WINDOW_SIZE, &count, 1000) || !count) {
            // failed read, dont care
            continue;
        }

        // just for debug
        // ESP_LOGI(TAG, "count: %" PRIu32, count);
        // for(int i = 0; i < count; i++) {
        //     if(data[i].valid) {
        //         ESP_LOGI(TAG, "adc%d, channel: %d, value: %" PRIu32, data[i].unit + 1, data[i].channel,
        //                 data[i].raw_data);
        //     }
        // }

        // in milivolts
        int ampl = calc_ampl(data, count);
        if(ampl) {
            ESP_LOGI(TAG, "measured ampl %dmV", ampl);
            // based on a previous control voltage and the measured output we calculate next control voltage
            update_control(ampl);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void
regulator_init(void) {
    init_adc();
    init_dac();

    int success = xTaskCreate(task, "regulator", 4096, NULL, 2, &g.task_handle);

    regulator_enable(800, 1000);
    ASSERT(success);
}

void
regulator_enable(int target, int freq) {
    if(g.enabled) {
        return;
    }

    // we want 256 times greater frequency of samling than the frequency of the signal. this assures we have enough data
    // and that it is tighly packed in a single period
    freq = CLAMP(256 * freq, SOC_ADC_SAMPLE_FREQ_THRES_LOW, SOC_ADC_SAMPLE_FREQ_THRES_HIGH);

    g.enabled = true;
    g.target = target;

    // starting value, middle of the interval
    g.control = 1500;

    adc_continuous_config_t cont_conf = {
            .pattern_num = 1,
            .adc_pattern = (adc_digi_pattern_config_t[]){{
                    .atten = ADC_ATTEN,
                    .channel = ADC_CHANNEL,
                    .unit = ADC_UNIT,
                    .bit_width = ADC_BITWIDTH,
            }},
            .sample_freq_hz = freq,
            .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_continuous_config(g.cont_handle, &cont_conf));
    ESP_ERROR_CHECK(adc_continuous_start(g.cont_handle));
}

void
regulator_disable(void) {
    if(!g.enabled) {
        return;
    }

    ESP_ERROR_CHECK(adc_continuous_stop(g.cont_handle));
    g.enabled = false;
}
