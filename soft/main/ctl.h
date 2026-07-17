#ifndef CTL_H
#define CTL_H

#include "esp_err.h"

typedef enum ctl_signal_type {
    CTL_SIGNAL_TYPE_NONE = 0,
    CTL_SIGNAL_TYPE_SINE,
    CTL_SIGNAL_TYPE_RECT,
    CTL_SIGNAL_TYPE_TRIANGLE,
    _CTL_SIGNAL_TYPE_COUNT,
} ctl_signal_type_t;

typedef struct ctl_params {
    int freq;
    float symmetry;
    float offset;
} ctl_params_t;

extern const char *ctl_signal_type_to_string[_CTL_SIGNAL_TYPE_COUNT];

void
ctl_init(void);

esp_err_t
ctl_enable(ctl_signal_type_t type, ctl_params_t *params);

esp_err_t
ctl_disable(void);

bool
ctl_is_enabled(void);

#endif
