#include "context.h"

#include <string.h>

symbol_t *get_symbol(context_t *ctx, str name) {
    const ast_node_t *scope = ctx->cur_scope;
    while (scope != NULL) {
        const symbol_tbl_t *symbol_tbl;
        if (scope->node_type == AST_STMT_COMPOUND) {
            symbol_tbl = &(((const ast_stmt_compound_t*) scope)->symbol_tbl);
        } else if (scope->node_type == AST_ROOT) {
            symbol_tbl = &(((const ast_root_t*) scope)->symbol_tbl);
        }

        symbol_t *symbol;
        HASH_MAP_GET(*symbol_tbl, name, symbol, str, symbol_t*, strcmp);
        if ((symbol->type & SYM_NOTEXIST) == 0) {
            return symbol;
        }

        scope = scope->parent;
    }

    return NULL;
}
