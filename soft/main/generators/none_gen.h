#ifndef NONE_GEN_H
#define NONE_GEN_H

#include "generators/gen.h"

// we might want to generate just a constant dc signal, so this implementation just does nothing
typedef struct none_gen {
    gen_t base_gen;
} none_gen_t;

void
none_gen_init(none_gen_t *gen);

#endif
