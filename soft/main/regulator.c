#include "regulator.h"

// TODO: check if this is right
#define ADC_MAX_VALUE (1024)

#define REGULATION_CONSTANT (0.05)

// TODO: investigate how this should be done
float
regulator_propose_next(float target, float prev, float meas) {
    return prev + (target - meas) * REGULATION_CONSTANT;
}
