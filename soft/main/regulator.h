#ifndef REGULATOR_H
#define REGULATOR_H

#include "util/ints.h"

float
regulator_propose_next(float target, float prev, float meas);

#endif
