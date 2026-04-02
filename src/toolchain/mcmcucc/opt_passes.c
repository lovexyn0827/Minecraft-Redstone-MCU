#include "opt_passes.h"

// *********** Constant Folding ***********

ast_node_t *eval_constexpr(ast_node_t *node) {
    debug("%d\n", node->node_type);
    return NULL;
}

void fold_constants(context_t *ctx) {
    for_each_node((ast_node_t*) &(ctx->ast.root), eval_constexpr, eval_constexpr);
}

// *********** Main Procedure ***********

void optimize_ast(context_t *ctx) {
    fold_constants(ctx);
}
