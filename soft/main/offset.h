#ifndef OFFSET_H
#define OFFSET_H

#include "esp_err.h"

void
offset_init(void);

esp_err_t
offset_enable(float value);

void
offset_disable(void);

#endif
