#include "parser.h"

#include "context.h"
#include "ast.h"

void error_on_token() {
}

// *********** Token streamizer ***********

bool inbounds(read_head_t *ptr) {
    return ptr->cur_pos < ptr->end_pos;
}

bool inbounds_after_n(read_head_t *ptr, uint_t offset) {
    return (ptr->cur_pos + offset) < ptr->end_pos;
}

void assert_inbounds(read_head_t *ptr) {
    if (!inbounds(ptr)) {
        error("Unexpected end of input.\n");
    }
}

const token_t *peek_current(read_head_t *ptr) {
    return inbounds(ptr) ? ptr->base[ptr->cur_pos] : NULL;
}

const token_t *peek_current_plus_n(read_head_t *ptr, uint_t offset) {
    return inbounds_after_n(ptr, offset) ? ptr->base[ptr->cur_pos + offset] : NULL;
}

const token_t *read_current(read_head_t *ptr) {
    // printf(peek_current(ptr));
    return inbounds(ptr) ? ptr->base[ptr->cur_pos++] : NULL;
}

void skip_current(read_head_t *ptr) {
    // printf(peek_current(ptr));
    if (inbounds(ptr)) (ptr->cur_pos)++;
}

// *********** Expressions ***********

// primary-expr	:= identifier | constant | ( expr )

bool parse_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest);

uint_t parse_constant_value(const token_t *token) {
}

bool parse_primary_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    const token_t *first_token = read_current(ctx->ptr);
    switch (first_token->type) {
    case TOKEN_CONST_BIN:
    case TOKEN_CONST_DEC:
    case TOKEN_CONST_HEX:
    case TOKEN_CONST_CHAR:
        ast_expr_constant_t *new_node = (ast_expr_constant_t *) malloc(sizeof(ast_expr_constant_t));
        new_node->node_type = AST_EXPR_CONST;
        new_node->parent = parent;
        new_node->constant = true;
        new_node->lvalue = false;
        new_node->value = parse_constant_value(first_token);
        break;
    default:
        return parse_expr(ctx, parent, dest);
    }
}

// *********** Main Procedure ***********

// compile-unit	:= extern-decl | compile-unit
// extern-decl	:= func-def | decl
// func-def		:= decl-spec declarator decl-list? comp-stmt
// decl-list	:= decl | decl-list decl

void parse(context_t *ctx, ast_t *ast) {

}
