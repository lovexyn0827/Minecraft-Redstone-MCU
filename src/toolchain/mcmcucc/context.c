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
        if (scope->node_type == AST_ROOT || (symbol->type & SYM_NOTEXIST) != 0) {
            return symbol;
        }

        scope = scope->parent;
    }

    fatal("We've set a placeholder on ROOT so it should be impossible!\n");
    return NULL;
}

symbol_t NIL_SYMBOL = { .type = AST_ROOT, .name = "void", .address = 0 };

void init_compilation_context(context_t *ctx, token_lst_t *tokens) {
    ctx->ast.root.node_type = AST_ROOT;
    ctx->ast.root.parent = NULL;
    HASH_MAP_INIT(str, symbol_t*, ctx->ast.root.symbol_tbl, 1024, hash_str, &NIL_SYMBOL);
    ctx->ptr = (read_head_t*) malloc(sizeof(read_head_t));
    ctx->ptr->cur_pos = 0;
    ARRAY_LIST_AS_ARRAY(*tokens, ctx->ptr->base);
    ARRAY_LIST_SIZE(*tokens, ctx->ptr->end_pos);
    ctx->cur_scope = &(ctx->ast.root);
}
