#include "context.h"

#include <string.h>

symbol_t *get_symbol(context_t *ctx, str name) {
    ast_node_t *scope = ctx->cur_scope;
    while (scope != NULL) {
        symbol_t *symbol;
        HASH_MAP_GET(scope->symbol_tbl, name, symbol, str, symbol_t*, strcmp);
        if ((symbol->type & SYM_NOTEXIST) == 0) {
            return symbol;
        }

        scope = scope->parent;
    }

    return NULL;
}
