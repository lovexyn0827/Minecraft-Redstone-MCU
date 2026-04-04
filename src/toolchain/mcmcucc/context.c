#include "context.h"

#include <string.h>

symbol_tbl_t *get_symbol_table(ast_node_t *scope) {
    switch (scope->node_type) {
    case AST_STMT_COMPOUND:
        return &(((ast_stmt_compound_t*) scope)->symbol_tbl);
    case AST_ROOT:
        return &(((ast_root_t*) scope)->symbol_tbl);
    case AST_FUNC_IMPL:
        return &(((ast_function_impl_t*) scope)->symbol_tbl);
    case AST_DECL_DRCT_FN:
        return &(((ast_decl_direct_function_t*) scope)->symbol_tbl);
    case AST_STMT_FORDECL:
        return &(((ast_stmt_fordecl_t*) scope)->symbol_tbl);
    default:
        return NULL;
    }
}

symbol_t *get_symbol(context_t *ctx, str name) {
    debug("Finding symbol %s\n", name);
    const ast_node_t *scope = ctx->cur_scope;
    while (scope != NULL) {
        const symbol_tbl_t *symbol_tbl = get_symbol_table((ast_node_t*) scope);
        if (symbol_tbl == NULL) {
            scope = scope->parent;
            continue;
        }

        HASH_MAP_TRAVERSE(*symbol_tbl, str, symbol_t *, str n, symbol_t *s, uint_t h, {
            debug("%s @ %d ==> Symbol [ %d ]\n", n, h, s->type);
        })
        symbol_t *symbol;
        HASH_MAP_GET(*symbol_tbl, name, symbol, str, symbol_t*, str_equal);
        if (scope->node_type == AST_ROOT || symbol->type != SYM_NOTEXIST) {
            return symbol;
        }

        scope = scope->parent;
    }

    fatal("We've set a placeholder on ROOT so it should be impossible!\n");
    return NULL;
}

void register_symbol(context_t *ctx, symbol_t *symb) {
    symbol_tbl_t *symbol_tbl = get_symbol_table(ctx->cur_scope);
    if (symbol_tbl == NULL) {
        fatal("This should never happen as we are ensuring cur_scope to denote a vaild scope node!\n");
    }

    HASH_MAP_PUT(*symbol_tbl, symb->name, symb, str, symbol_t*, str_equal)
}

symbol_t *register_declared_symbol(context_t *ctx, str name, const ast_decl_t *decl) {
    symbol_t *symb = (symbol_t*) malloc(sizeof(symbol_t));
    symb->decl = decl;
    symb->name = name;
    switch (decl->node_type) {
    case AST_DECL_DRCT_VAR:
        const ast_decl_direct_variable_t *var_decl = (const ast_decl_direct_variable_t*) decl;
        symb->immutable = var_decl->decl_type->immutable;
        symb->type = SYM_VARIABLE;
        symb->address = (obj_addr_t*) malloc(sizeof(obj_addr_t));
        symb->address->type = OBJ_ADDR_IM;
        symb->address->addr = NIL_OBJ_ADDR; // TODO Linkage
        break;
    case AST_DECL_DRCT_FN:
        const ast_decl_direct_function_t *fn_decl = (const ast_decl_direct_function_t*) decl;
        symb->immutable = fn_decl->decl_type->immutable;
        symb->type = SYM_FUNCTION;
        symb->address = (obj_addr_t*) malloc(sizeof(obj_addr_t));
        symb->address->type = OBJ_ADDR_IM;
        symb->address->addr = NIL_OBJ_ADDR; // TODO Linkage
        break;
    default:
        fatal("No way! It should never happen!\n");
    }

    register_symbol(ctx, symb);
    return symb;
}

symbol_t *register_label(context_t *ctx, str name) {
    if (ctx->cur_func == NULL) {
        return NULL;
    }

    symbol_t *symb = (symbol_t*) malloc(sizeof(symbol_t));
    symb->decl = NULL;
    symb->name = name;
    symb->immutable = true;
    symb->type = SYM_LABEL;
    symb->address = (obj_addr_t*) malloc(sizeof(obj_addr_t));
    symb->address->type = OBJ_ADDR_IM;
    symb->address->addr = NIL_OBJ_ADDR; // TODO Linkage
    HASH_MAP_PUT(ctx->cur_func->symbol_tbl, name, symb, str, symbol_t*, str_equal)
    return symb;
}

symbol_t NIL_SYMBOL = { .type = SYM_NOTEXIST, .name = "[Null]", .address = NULL };
symbol_t UNSPECIFIED_SYMBOL = { .type = SYM_NOTEXIST, .name = "[Unspecified]", .address = NULL };

void init_compilation_context(context_t *ctx, token_lst_t *tokens) {
    ctx->ast.root.node_type = AST_ROOT;
    ctx->ast.root.parent = NULL;
    ARRAY_LIST_INIT(const ast_node_t*, ctx->ast.root.children)
    HASH_MAP_INIT(str, symbol_t*, ctx->ast.root.symbol_tbl, 1024, hash_str, &NIL_SYMBOL);
    ctx->ptr = (read_head_t*) malloc(sizeof(read_head_t));
    ctx->ptr->cur_pos = 0;
    ARRAY_LIST_INIT(uint_t, ctx->ptr->marks);
    ARRAY_LIST_AS_ARRAY(*tokens, ctx->ptr->base);
    ARRAY_LIST_SIZE(*tokens, ctx->ptr->end_pos);
    ctx->cur_scope = (ast_node_t*) &(ctx->ast.root);
    ctx->cur_func = NULL;
}
