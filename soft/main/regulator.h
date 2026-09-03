#ifndef REGULATOR_H
#define REGULATOR_H

void
regulator_init(void);

// target is in milivolts
void
regulator_enable(int target, int freq);

void
regulator_disable(void);

#endif
