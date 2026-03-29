#include "opt_passes.h"

// *********** Constant Folding ***********

void fold_constants(context_t *ctx) {
}

// *********** Main Procedure ***********

void optimize_ast(context_t *ctx) {
    fold_constants(ctx);
}
