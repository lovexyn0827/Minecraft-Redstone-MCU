#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "common.h"
#include "array_list.h"
#include "map.h"
#include "ast.h"
#include "tokenizer.h"

typedef struct {
    const token_t **base;
    uint_t cur_pos;
    uint_t end_pos;
    ARRAY_LIST_TYPE(uint_t) marks;
} read_head_t;

typedef enum {
    SYM_FUNCTION = 1 << 0,
    SYM_VARIABLE = 1 << 1,
    SYM_LABEL = 1 << 2,
    SYM_NOTEXIST = 1 << 31
} symbol_type_t;

typedef struct symbol_t {
    symbol_type_t type;
    str name;
    const ast_decl_t *decl;
    bool immutable;
    uint_t address;
} symbol_t;

typedef struct context {
    read_head_t *ptr;
    ast_node_t *cur_scope;
    ast_t ast;
} context_t;

symbol_t *get_symbol(context_t *ctx, str name);
symbol_t *register_declared_symbol(context_t *ctx, str name, const ast_decl_t *decl);
void register_label(context_t *ctx, str name);

extern symbol_t NIL_SYMBOL;

void init_compilation_context(context_t *ctx, token_lst_t *tokens);

#endif // CONTEXT_H_INCLUDED
