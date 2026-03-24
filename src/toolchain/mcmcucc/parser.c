#include "parser.h"

#include "context.h"
#include "ast.h"

#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

extern uint_t error_cnt, warning_cnt;

void fatal_on_token(const token_t *token, str fmt, ...) {
    printf("On line %d:%d @ %s:\n", token->line_num, token->column_pos, token->token);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(-1);
}

void error_on_token(const token_t *token, str fmt, ...) {
    printf("On line %d:%d @ %s:\n", token->line_num, token->column_pos, token->token);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    error_cnt++;
}

void warn_on_token(const token_t *token, str fmt, ...) {
    printf("On line %d:%d @ %s:\n", token->line_num, token->column_pos, token->token);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    warning_cnt++;
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
        fatal("Unexpected end of input.\n");
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
    assert_inbounds(ptr);
    debug("Skipping token %s\n", peek_current(ptr)->token);
    return inbounds(ptr) ? ptr->base[ptr->cur_pos++] : NULL;
}

void unread_prev(read_head_t *ptr) {
    if (ptr->cur_pos > 0) {
        ptr->cur_pos--;
    }

    debug("Unread token %s\n", peek_current(ptr)->token);
}

void skip_current(read_head_t *ptr) {
    debug("Skipping token %s\n", peek_current(ptr)->token);
    if (inbounds(ptr)) (ptr->cur_pos)++;
}

void verify_and_skip_current(read_head_t *ptr, token_type_t type) {
    if (peek_current(ptr)->type != type) {
        error_on_token(peek_current(ptr), "Expected token %d but got %d!\n", type, peek_current(ptr)->type);
        return;
    }

    skip_current(ptr);
}

void mark_current(read_head_t *ptr) {
    ARRAY_LIST_APPEND(ptr->marks, ptr->cur_pos, uint_t)
}

void return_marked(read_head_t *ptr) {
    uint_t pos;
    ARRAY_LIST_GET_TAIL(ptr->marks, pos)
    ptr->cur_pos = pos;
}

// *********** Expressions ***********

// primary-expr	:= identifier | constant | ( expr )

bool parse_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest);

uint_t parse_bin_constant(str str_rep) {
    uint_t value = 0;
    for (int i = 2; i < strlen(str_rep); i++) {
        value = (value << 1) | (str_rep[i] & ~'0');
    }

    return value;
}

uint_t parse_dec_constant(str str_rep) {
    uint_t value = 0;
    for (int i = 0; i < strlen(str_rep); i++) {
        value = (value * 10) + (str_rep[i] & ~'0');
    }

    return value;
}

uint_t parse_hex_constant(str str_rep) {
    uint_t value = 0;
    for (int i = 2; i < strlen(str_rep); i++) {
        char_t digit = str_rep[i];
        value = (value << 4) + ((digit > '9') ? toupper(digit) - 'A' : digit & ~'0');
    }

    return value;
}

uint_t parse_char_constant(str str_rep) {
    if (str_rep[1] == '\\') {
        switch (str_rep[2]) {
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '\?':
            return '\?';
        case '\\':
            return '\\';
        case '\a':
            return '\a';
        case '\b':
            return '\b';
        case '\f':
            return '\f';
        case '\n':
            return '\n';
        case '\r':
            return '\r';
        case '\t':
            return '\t';
        case '\v':
            return '\v';
        default:
            return str_rep[2];
        }
    } else {
        return str_rep[1];
    }
}

uint_t parse_constant_value(const token_t *token) {
    str str_rep = token->token;
    switch (token->type) {
    case TOKEN_CONST_BIN:
        return parse_bin_constant(str_rep);
    case TOKEN_CONST_DEC:
        return parse_dec_constant(str_rep);
    case TOKEN_CONST_HEX:
        return parse_hex_constant(str_rep);
    case TOKEN_CONST_CHAR:
        return parse_char_constant(str_rep);
    default:
        fatal("No way! How did we get there?");
        return 0;
    }
}

bool parse_primary_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: primary-expr\n");
    const token_t *first_token = read_current(ctx->ptr);
    switch (first_token->type) {
    case TOKEN_CONST_BIN:
    case TOKEN_CONST_DEC:
    case TOKEN_CONST_HEX:
    case TOKEN_CONST_CHAR:
        ast_expr_constant_t *const_node = (ast_expr_constant_t *) malloc(sizeof(ast_expr_constant_t));
        const_node->node_type = AST_EXPR_CONST;
        const_node->parent = parent;
        const_node->constant = true;
        const_node->lvalue = false;
        const_node->value = parse_constant_value(first_token);
        *dest = (ast_expr_t*) const_node;;
        return true;
    case TOKEN_IDENTIFIER:
        ast_expr_symbol_t *sym_node = (ast_expr_symbol_t *) malloc(sizeof(ast_expr_symbol_t));
        sym_node->node_type = AST_EXPR_SYMBOL;
        sym_node->parent = parent;
        symbol_t *symb = get_symbol(ctx, first_token->token);
        sym_node->symbol = symb;
        switch (symb->type) {
        case SYM_FUNCTION:
        case SYM_LABEL:
            sym_node->constant = true;
            sym_node->lvalue = false;
            *dest = (ast_expr_t*) sym_node;
            return true;
        case SYM_VARIABLE:
            sym_node->constant = false;
            sym_node->lvalue = true;
            *dest = (ast_expr_t*) sym_node;
            return true;
        case SYM_NOTEXIST:
            error_on_token(first_token, "Undefined symbol: %s\n", first_token->token);
            free(sym_node);
            return true;
        default:
            fatal_on_token(first_token, "Unrecognized symbol type %d\n", symb->type);
        }
    case TOKEN_PUNCT_L_P:
        // We've skipped '('
        bool is_expr = parse_expr(ctx, parent, dest);
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P);
        return is_expr;
    default:
        error_on_token(first_token, "Unrecognized premable for prim-expr: %d\n", first_token->type);
        return false;
    }
}

// postfix-expr	:= primary-expr
//                     | postfix-expr [ expr ]
//                     | postfix-expr ( arg-expr-list? )
//                     | postfix-expr ++
//                     | postfix-expr --

bool parse_assign_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest);

bool parse_postfix_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: postfix-expr\n");
    ast_expr_t *left_hand;
    if (!parse_primary_expr(ctx, NULL, &left_hand)) {
        return false;
    }

    const token_t *postfix_start = peek_current(ctx->ptr);
    switch (postfix_start->type) {
    case TOKEN_PUNCT_L_SP:
        // Array subscript
        skip_current(ctx->ptr);
        ast_expr_binary_t *array_subscr_node = (ast_expr_binary_t*) malloc(sizeof(ast_expr_binary_t));
        array_subscr_node->node_type = AST_EXPR_BINARY;
        array_subscr_node->parent = parent;
        array_subscr_node->lvalue = true;
        array_subscr_node->op = BOP_SUBSCR;
        array_subscr_node->left_opnd = left_hand;
        parse_expr(ctx, (ast_node_t*) array_subscr_node, (ast_expr_t**) &(array_subscr_node->right_opnd));
        array_subscr_node->constant = left_hand->constant && array_subscr_node->right_opnd->constant;
        left_hand->parent = (ast_node_t*) array_subscr_node;
        *dest = (ast_expr_t*) array_subscr_node;
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_SP); // Skip ']'
        return true;
    case TOKEN_PUNCT_L_P:
        // Function call
        skip_current(ctx->ptr);
        ast_expr_call_t *func_call_node = (ast_expr_call_t*) malloc(sizeof(ast_expr_call_t));
        func_call_node->node_type = AST_EXPR_CALL;
        func_call_node->parent = parent;
        func_call_node->lvalue = false;
        func_call_node->constant = false;
        func_call_node->function_addr = left_hand;
        ARRAY_LIST_INIT(const ast_expr_t*, func_call_node->arguments);
        const token_t *cur_token;
        while ((cur_token = peek_current(ctx->ptr))->type != TOKEN_PUNCT_R_P) {
            ast_expr_t *arg;
            parse_assign_expr(ctx, (ast_node_t*) func_call_node, &arg);
            ARRAY_LIST_APPEND(func_call_node->arguments, arg, const ast_expr_t*);
            if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_COMMA) {
                skip_current(ctx->ptr);
            }
        }

        left_hand->parent = (ast_node_t*) func_call_node;
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P); // Skip ')'
        *dest = (ast_expr_t*) func_call_node;
        return true;
    case TOKEN_PUNCT_INC:
    case TOKEN_PUNCT_DEC:
        // Get and increment
        // Get and decrement
        skip_current(ctx->ptr);
        ast_expr_unary_t *getinc_node = (ast_expr_unary_t*) malloc(sizeof(ast_expr_unary_t));
        getinc_node->node_type = AST_EXPR_UNARY;
        getinc_node->parent = parent;
        getinc_node->op = postfix_start->type == TOKEN_PUNCT_INC ? UOP_GETINC : UOP_GETDEC;
        getinc_node->constant = false;
        getinc_node->lvalue = false;
        getinc_node->opnd = left_hand;
        left_hand->parent = (ast_node_t*) getinc_node;
        *dest = (ast_expr_t*) getinc_node;
        if (!left_hand->lvalue) {
            error_on_token(postfix_start, "Operand of ++ or -- must be lvalue!");
            return true;
        }

        return true;
    default:
        // Unexpected
        *dest = left_hand;
        return true;
    }
}

// unary-expr   := postfix-expr
// 					| ++ unary-expr
// 					| -- unary-expr
// 					| unary-op unary-expr
// 					| sizeof unary-expr
// 					| sizeof ( type-name )
// unary-op		:= & * + - ~ !

bool parse_typename(context_t *ctx, ast_node_t *parent, ast_typename_t **dest);

bool parse_unary_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: unary-expr\n");
    const token_t *prefix = peek_current(ctx->ptr);
    switch (prefix->type) {
    case TOKEN_PUNCT_INC:
    case TOKEN_PUNCT_DEC:
        // Increment and get
        // Decrement and get
        skip_current(ctx->ptr);
        ast_expr_unary_t *getinc_node = (ast_expr_unary_t*) malloc(sizeof(ast_expr_unary_t));
        getinc_node->node_type = AST_EXPR_UNARY;
        getinc_node->parent = parent;
        getinc_node->op = prefix->type == TOKEN_PUNCT_INC ? UOP_INCGET : UOP_DECGET;
        getinc_node->constant = false;
        getinc_node->lvalue = false;
        parse_unary_expr(ctx, (ast_node_t*) getinc_node, (ast_expr_t**) &(getinc_node->opnd));
        *dest = (ast_expr_t*) getinc_node;
        if (!getinc_node->opnd->lvalue) {
            error_on_token(prefix, "Operand of ++ or -- must be lvalue!");
            return true;
        }

        return true;
    case TOKEN_PUNCT_AND:
    case TOKEN_PUNCT_STAR:
    case TOKEN_PUNCT_PLUS:
    case TOKEN_PUNCT_MINUS:
    case TOKEN_PUNCT_NEG:
    case TOKEN_PUNCT_NOT:
        skip_current(ctx->ptr);
        ast_expr_unary_t *unary_node = (ast_expr_unary_t*) malloc(sizeof(ast_expr_unary_t));
        unary_node->node_type = AST_EXPR_UNARY;
        unary_node->parent = parent;
        unary_node->lvalue = false;
        switch (prefix->type) {
        case TOKEN_PUNCT_AND:
            unary_node->op = UOP_ADDRESSOF;
            break;
        case TOKEN_PUNCT_STAR:
            unary_node->op = UOP_DEREFERENCE;
            unary_node->lvalue = true;
            break;
        case TOKEN_PUNCT_PLUS:
            unary_node->op = UOP_POSITIVE;
            break;
        case TOKEN_PUNCT_MINUS:
            unary_node->op = UOP_NEGATIVATE;
            break;
        case TOKEN_PUNCT_NEG:
            unary_node->op = UOP_BITWISE_NOT;
            break;
        case TOKEN_PUNCT_NOT:
            unary_node->op = UOP_LOGICAL_NOT;
            break;
        default:
            fatal("No way! How did we get there?");
        }

        if (!parse_unary_expr(ctx, (ast_node_t*) unary_node, (ast_expr_t**) &(unary_node->opnd))) {
            return false;
        }

        unary_node->constant = unary_node->opnd->constant;
        *dest = (ast_expr_t*) unary_node;
        return true;
        // Unary ops
    case TOKEN_KW_SIZEOF:
        skip_current(ctx->ptr);
        ast_expr_t *sizeof_expr;
        if (parse_unary_expr(ctx, NULL, &sizeof_expr)) {
            ast_expr_sizeof_expr_t *sizeof_node = (ast_expr_sizeof_expr_t *) malloc(sizeof(ast_expr_sizeof_expr_t));
            sizeof_node->node_type = AST_EXPR_EXPRSZ;
            sizeof_node->parent = parent;
            sizeof_node->constant = true;
            sizeof_node->lvalue = false;
            sizeof_expr->parent = (ast_node_t*) sizeof_node;
            *dest = (ast_expr_t*) sizeof_node;
            return true;
        } else {
            ast_expr_sizeof_type_t *sizeof_node = (ast_expr_sizeof_type_t *) malloc(sizeof(ast_expr_sizeof_type_t));
            sizeof_node->node_type = AST_EXPR_TYPESZ;
            sizeof_node->parent = parent;
            sizeof_node->constant = true;
            sizeof_node->lvalue = false;
            verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_L_P); // Skip '('
            parse_typename(ctx, (ast_node_t*) sizeof_node, (ast_typename_t**) &(sizeof_node->sizeof_type));
            verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P); // Skip ')'
            *dest = (ast_expr_t*) sizeof_node;
            return true;
        }
    default:
        return parse_postfix_expr(ctx, parent, dest);
    }
}

// cast-expr	:= unary-expr
// 					| \( type-name \) cast-expr

bool parse_cast_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    // FIXME
    debug(" ==> Parsing: cast-expr\n");
    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        ast_expr_cast_t *cast_node = (ast_expr_cast_t*) malloc(sizeof(ast_expr_cast_t));
        cast_node->lvalue = false;
        cast_node->node_type = AST_EXPR_CAST;
        cast_node->parent = parent;
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_L_P); // Skip '('
        if (!parse_typename(ctx, (ast_node_t*) cast_node, (ast_typename_t**) &(cast_node->cast_to))) {
            // It is prim-expr '(E)' instead of '(T) E'
            free(cast_node);
            unread_prev(ctx->ptr);
            return parse_unary_expr(ctx, parent, dest);
        }

        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P); // Skip ')'
        if (!parse_cast_expr(ctx, (ast_node_t*) cast_node, (ast_expr_t**) &(cast_node->opnd))) {
            error_on_token(peek_current(ctx->ptr), "Expected cast-expr\n");
            free(cast_node);
            return false;
        }

        cast_node->constant = cast_node->opnd->constant;
        *dest = (ast_expr_t*) cast_node;
        return true;
    } else {
        return parse_unary_expr(ctx, parent, dest);
    }
}

// binary-expr  := opnd-expr binary-op opnd-expr

typedef bool (*phrase_parser) (context_t *, ast_node_t *, ast_expr_t **);

bool parse_binary_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest,
                         binary_op_t (*token2op)(const token_t *), phrase_parser opnd_parser) {
    ast_expr_t *left_opnd;
    // Apparently, we cannot call parse_multiplicative_expr() unconditionally
    // We have to push something forward beforehand at least.
    // Also, we note that mult-expr := cast-expr (mult-op cast-expr)*, which leads to following algorithm
    if (!opnd_parser(ctx, parent, &left_opnd)) {
        return false;
    }

    while (true) {
        binary_op_t op = token2op(peek_current(ctx->ptr));
        if (op == BOP_UNRECOGNIZED) {
            *dest = left_opnd;
            return true;
        }

        skip_current(ctx->ptr); // Skip multiplicative operator
        ast_expr_binary_t *new_node = (ast_expr_binary_t*) malloc(sizeof(ast_expr_binary_t));
        new_node->node_type = AST_EXPR_BINARY;
        new_node->op = op;
        new_node->parent = parent;
        new_node->left_opnd = left_opnd;
        left_opnd->parent = (ast_node_t*) new_node;
        new_node->lvalue = false;
        opnd_parser(ctx, (ast_node_t*) new_node, (ast_expr_t**) &(new_node->right_opnd));
        new_node->constant = new_node->left_opnd->constant && new_node->right_opnd->constant;
        left_opnd = (ast_expr_t*) new_node;
    }
}

// mult-expr	:= cast-expr
// 					| mult-expr * cast-expr
// 					| mult-expr / cast-expr
// 					| mult-expr % cast-expr

binary_op_t mult_op_token_to_op(const token_t *op_token) {
    switch (op_token->type) {
    case TOKEN_PUNCT_STAR:
        return BOP_MUL;
    case TOKEN_PUNCT_SLASH:
        return BOP_DIV;
    case TOKEN_PUNCT_PERCENT:
        return BOP_MOD;
    default:
        return BOP_UNRECOGNIZED;
    }
}

bool parse_multiplicative_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: mult-expr\n");
    return parse_binary_expr(ctx, parent, dest, mult_op_token_to_op, parse_cast_expr);
}

// additive-expr    := mult-expr
// 					| additive-expr + mult-expr
// 					| additive-expr - mult-expr

binary_op_t additive_op_token_to_op(const token_t *op_token) {
    switch (op_token->type) {
    case TOKEN_PUNCT_PLUS:
        return BOP_ADD;
    case TOKEN_PUNCT_MINUS:
        return BOP_SUB;
    default:
        return BOP_UNRECOGNIZED;
    }
}

bool parse_additive_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: add-expr\n");
    return parse_binary_expr(ctx, parent, dest, additive_op_token_to_op, parse_multiplicative_expr);
}

// shift-expr		:= additive-expr
// 					| shift-expr << additive-expr
// 					| shift-expr >> additive-expr

binary_op_t shift_op_token_to_op(const token_t *op_token) {
    switch (op_token->type) {
    case TOKEN_PUNCT_SHL:
        return BOP_SHL;
    case TOKEN_PUNCT_SHR:
        return BOP_SHR;
    default:
        return BOP_UNRECOGNIZED;
    }
}

bool parse_shift_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: shift-expr\n");
    return parse_binary_expr(ctx, parent, dest, shift_op_token_to_op, parse_additive_expr);
}

// relation-expr	:= shift-expr
// 					| relation-expr < shift-expr
// 					| relation-expr > shift-expr
// 					| relation-expr <= shift-expr
//					| relation-expr >= shift-expr

binary_op_t relation_op_token_to_op(const token_t *op_token) {
    switch (op_token->type) {
    case TOKEN_PUNCT_LT:
        return BOP_LT;
    case TOKEN_PUNCT_GT:
        return BOP_GT;
    case TOKEN_PUNCT_LE:
        return BOP_LE;
    case TOKEN_PUNCT_GE:
        return BOP_GE;
    default:
        return BOP_UNRECOGNIZED;
    }
}

bool parse_relation_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: relation-expr\n");
    return parse_binary_expr(ctx, parent, dest, relation_op_token_to_op, parse_shift_expr);
}

// equality-expr	:= relation-expr
// 					| equality-expr == relation-expr
// 					| equality-expr != relation-expr

binary_op_t equality_op_token_to_op(const token_t *op_token) {
    switch (op_token->type) {
    case TOKEN_PUNCT_EQ:
        return BOP_EQ;
    case TOKEN_PUNCT_NE:
        return BOP_NE;
    default:
        return BOP_UNRECOGNIZED;
    }
}

bool parse_equality_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: equality-expr\n");
    return parse_binary_expr(ctx, parent, dest, equality_op_token_to_op, parse_relation_expr);
}

// and-expr		:= equality-expr
// 					| and-expr & equality-expr

binary_op_t and_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_AND) ? BOP_AND : BOP_UNRECOGNIZED;
}

bool parse_and_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: and-expr\n");
    return parse_binary_expr(ctx, parent, dest, and_op_token_to_op, parse_equality_expr);
}

// xor-expr		:= and-expr
// 					| xor-expr ^ and-expr

binary_op_t xor_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_XOR) ? BOP_XOR : BOP_UNRECOGNIZED;
}

bool parse_xor_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: xor-expr\n");
    return parse_binary_expr(ctx, parent, dest, xor_op_token_to_op, parse_and_expr);
}

// or-expr			:= xor-expr
// 					| or-expr \| xor-expr

binary_op_t or_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_OR) ? BOP_OR : BOP_UNRECOGNIZED;
}

bool parse_or_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: or-expr\n");
    return parse_binary_expr(ctx, parent, dest, or_op_token_to_op, parse_xor_expr);
}

// land-expr		:= or-expr
// 					| land-expr && or-expr

binary_op_t land_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_LAND) ? BOP_LAND : BOP_UNRECOGNIZED;
}

bool parse_land_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: land-expr\n");
    return parse_binary_expr(ctx, parent, dest, land_op_token_to_op, parse_or_expr);
}

// lor-expr		:= land-expr
// 					| lor-expr \|\| land-expr

binary_op_t lor_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_LAND) ? BOP_LAND : BOP_UNRECOGNIZED;
}

bool parse_lor_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: lor-expr\n");
    return parse_binary_expr(ctx, parent, dest, lor_op_token_to_op, parse_land_expr);
}

// cond-expr		:= lor-expr
//					| lor-expr likelyhood-spec? \? expr : cond-expr

bool parse_likelyhood_spec(context_t *ctx) {
    switch (peek_current(ctx->ptr)->type) {
    case TOKEN_KW_LIKELY:
        skip_current(ctx->ptr);
        return true;
    case TOKEN_KW_UNLIKELY:
        skip_current(ctx->ptr);
        return false;
    default:
        return true;
    }
}

bool parse_cond_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: cond-expr\n");
    ast_expr_t *left_opnd;
    if (!parse_lor_expr(ctx, parent, &left_opnd)) {
        return false;
    }

    bool likely_true = parse_likelyhood_spec(ctx);
    if (peek_current(ctx->ptr)->type != TOKEN_PUNCT_QUESTION) {
        // bare lor-expr
        *dest = left_opnd;
        return true;
    }

    ast_expr_cond_t *cond_node = (ast_expr_cond_t*) malloc(sizeof(ast_expr_cond_t));
    cond_node->node_type = AST_EXPR_COND;
    cond_node->cond = left_opnd;
    parse_expr(ctx, (ast_node_t*) cond_node, (ast_expr_t**) &(cond_node->if_true));
    verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_COLON);   // Skip ':'
    parse_cond_expr(ctx, (ast_node_t*) cond_node, (ast_expr_t**) &(cond_node->if_false));
    cond_node->lvalue = cond_node->if_false->lvalue && cond_node->if_true->lvalue;
    cond_node->constant = cond_node->cond->constant && cond_node->if_false->constant && cond_node->if_true->constant;
    cond_node->likely_true = likely_true;
    cond_node->parent = parent;
    *dest = (ast_expr_t*) cond_node;
    return true;
}

// assign-expr		:= cond-expr
// 					| unary-expr assign-op assign-expr
// assign-op		:= = | *= | /= | %= | += | -= | <<= | >>= | &= | ^= | \|= | <\| | <&

bool parse_assign_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: assign-expr\n");
    ast_expr_t *left_opnd;
    if (parse_cond_expr(ctx, parent, &left_opnd)) {
        *dest = left_opnd;
        return true;
    }

    ast_expr_t *assign_dest;
    parse_unary_expr(ctx, parent, &assign_dest);
    assign_op_t assign_op;
    const token_t *assign_op_token = read_current(ctx->ptr);
    switch (assign_op_token->type) {
    case TOKEN_PUNCT_ASSIGN:
        assign_op = AOP_EQ;
        break;
    case TOKEN_PUNCT_MULEQ:
        assign_op = AOP_MULEQ;
        break;
    case TOKEN_PUNCT_DIVEQ:
        assign_op = AOP_DIVEQ;
        break;
    case TOKEN_PUNCT_MODEQ:
        assign_op = AOP_MODEQ;
        break;
    case TOKEN_PUNCT_PLUSEQ:
        assign_op = AOP_ADDEQ;
        break;
    case TOKEN_PUNCT_MINUSEQ:
        assign_op = AOP_SUBEQ;
        break;
    case TOKEN_PUNCT_SHLEQ:
        assign_op = AOP_SHLEQ;
        break;
    case TOKEN_PUNCT_SHREQ:
        assign_op = AOP_SHREQ;
        break;
    case TOKEN_PUNCT_ANDEQ:
        assign_op = AOP_ANDEQ;
        break;
    case TOKEN_PUNCT_XOREQ:
        assign_op = AOP_XOREQ;
        break;
    case TOKEN_PUNCT_OREQ:
        assign_op = AOP_OREQ;
        break;
    case TOKEN_PUNCT_SET:
        assign_op = AOP_SET;
        break;
    case TOKEN_PUNCT_CLR:
        assign_op = AOP_CLR;
        break;
    default:
        unread_prev(ctx->ptr);
        return true;
    }

    ast_expr_assign_t *assign_node = (ast_expr_assign_t*) malloc(sizeof(ast_expr_assign_t));
    assign_node->node_type = AST_EXPR_ASSIGN;
    assign_node->parent = parent;
    assign_node->constant = false;
    assign_node->op = assign_op;
    assign_node->dest = assign_dest;
    parse_assign_expr(ctx, (ast_node_t*) assign_node, (ast_expr_t**) &(assign_node->src));
    assign_node->lvalue = assign_node->src->lvalue;    // XXX
    if (!assign_dest->lvalue) {
        error_on_token(assign_op_token, "Destnation must be lvalue!\n");
    }
    return true;
}

// expr			:= assign-expr | expr , assign-expr

binary_op_t comma_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_COMMA) ? BOP_COMMA : BOP_UNRECOGNIZED;
}

bool parse_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    debug(" ==> Parsing: expr\n");
    return parse_binary_expr(ctx, parent, dest, comma_op_token_to_op, parse_assign_expr);
}

// *********** Declarations ***********

// pointer			:= * const? | * const? pointer

// Use with caution on memory leakage
void clone_typename_node(ast_typename_t **node) {
    size_t size;
    switch ((*node)->node_type) {
    case AST_TYPE_FUNCT:
        size = sizeof(ast_typename_funct_t);
        break;
    case AST_TYPE_PTR:
        size = sizeof(ast_typename_ptr_t);
        break;
    case AST_TYPE_PRIM:
        size = sizeof(ast_typename_prim_t);
        break;
    default:
        fatal("We're not supposed to be there anyway.");
    }

    void* dest = malloc(size);
    memcpy(dest, *node, size);
    *node = (ast_typename_t*) dest;
}

void check_for_duplicated_type_spec(context_t *ctx, uint_t flags) {
    if ((flags & (DECL_SPEC_INT8_T | DECL_SPEC_UINT8_T | DECL_SPEC_VOID)) != 0) {
        error_on_token(peek_current(ctx->ptr), "Duplicated type-specifier!\n");
    }
}

bool parse_decl_specifier(context_t *ctx, uint_t *dest) {
    debug(" ==> Parsing: decl-spec\n");
    uint_t flags = 0;
    while (true) {
        switch (peek_current(ctx->ptr)->type) {
        case TOKEN_KW_REGISTER:
            skip_current(ctx->ptr);
            flags |= DECL_SPEC_REGISTER;
            break;
        case TOKEN_KW_CONST:
            skip_current(ctx->ptr);
            flags |= DECL_SPEC_CONST;
            break;
        case TOKEN_KW_INLINE:
            skip_current(ctx->ptr);
            flags |= DECL_SPEC_INLINE;
            break;
        case TOKEN_KW_UINT8_T:
            skip_current(ctx->ptr);
            check_for_duplicated_type_spec(ctx, flags);
            flags |= DECL_SPEC_UINT8_T;
            break;
        case TOKEN_KW_INT8_T:
            skip_current(ctx->ptr);
            check_for_duplicated_type_spec(ctx, flags);
            flags |= DECL_SPEC_INT8_T;
            break;
        case TOKEN_KW_VOID:
            skip_current(ctx->ptr);
            check_for_duplicated_type_spec(ctx, flags);
            flags |= DECL_SPEC_VOID;
            break;
        default:
            if (flags == 0) return false;
            *dest = flags;
            return true;
        }
    }
}

// drct-abst-decl	:= ( abst-declarator )
// 					| drct-abst-decl? ( param-list? )

bool parse_param_list(context_t *ctx, param_list_t *dest);

// abst-declarator	:= pointer
// 					| pointer? drct-abst-decl

// What makes everthing tricky is that parents are types to derived from - implying a reversed parental relationship.
// Let A, B, P denote abst-decl, drct-abst-decl, pointer respectively
// Since A := P | P? B and B := (A) | B(Q), we have A := P | P? (A) (Q?)*
// And considering sematic constains, we have A := P | P? ( A ) | P? ( A ) ( Q? )
// (Functions ARE also types, but only pointers to functions can be returned.)
// Which means the resulting syntax tree is "linear" and the deepest node is well defined.
// Thus, we can omit intermediate B and treat branches in A as transformations for better clearity.
// Finally, we recognize leaves by A's consisting of only a P instance.
//
// Note that the recursion in function types is "evaluated after" function type derivations, or P before (Q) in B.
// E.g. in int * ( A' ) ( Q ), int is transformed into int*, then int * (Q), and then further transformeb by A'.
// Thus we must read Q first to complete function type derivation before recursion, which may reqiure one more pass.
// In the first of which we may push recursive derivations into a stack, and pop & apply in the second pass.

typedef enum type_derivation_type {
    DERIVATION_POINTER,
    DERIVATION_FUNCTION
} type_derivation_type_t;

typedef struct type_derivation {
    type_derivation_type_t type;
} type_derivation_t;

// We are including a series of continuous pointer derivations
typedef struct type_derivation_pointer {
    type_derivation_type_t type;
    ARRAY_LIST_TYPE(bool) constant;
} type_derivation_pointer_t;

typedef struct type_derivation_function {
    type_derivation_type_t type;
    param_list_t params;
} type_derivation_function_t;

// Parsing a series of continuous pointer type derivations
bool parse_pointer_decl(context_t *ctx, type_derivation_pointer_t **dest) {
    debug(" ==> Parsing: pointer\n");
    if (peek_current(ctx->ptr)->type != TOKEN_PUNCT_STAR) {
        return false;
    }

    type_derivation_pointer_t *ptr_derivation = (type_derivation_pointer_t*) malloc(sizeof(type_derivation_pointer_t));
    ptr_derivation->type = DERIVATION_POINTER;
    while (peek_current(ctx->ptr)->type == TOKEN_PUNCT_STAR) {
        skip_current(ctx->ptr);
        if (peek_current(ctx->ptr)->type == TOKEN_KW_CONST) {
            ARRAY_LIST_APPEND(ptr_derivation->constant, true, bool);
            skip_current(ctx->ptr);
        } else {
            ARRAY_LIST_APPEND(ptr_derivation->constant, false, bool);
        }
    }

    *dest = ptr_derivation;
    return true;
}

typedef ARRAY_LIST_TYPE(const type_derivation_t*) type_derivation_stack_t;

bool parse_abst_declarator_internal(context_t *ctx, type_derivation_stack_t *derivations) {
    debug(" ==> Parsing: abst-declarator\n");
    type_derivation_pointer_t *ptr_derivation;
    bool has_ptr_decl;
    has_ptr_decl = parse_pointer_decl(ctx, &ptr_derivation);
    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        // ( A ) part
        skip_current(ctx->ptr);
        if (!parse_abst_declarator_internal(ctx, derivations)) {
            if (has_ptr_decl) {
                return true;
            } else {
                return_marked(ctx->ptr);
                return false;
            }
        }

        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P);
    } else {
        if (has_ptr_decl) {
            // A := P and nothing follows
            ARRAY_LIST_APPEND(*derivations, ((type_derivation_t*) ptr_derivation), type_derivation_t*);
            return true;
        } else {
            return false;
        }
    }

    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        // A := P? ( A ) ( Q ) and we are parsing ( Q ) since P? ( A ) has already been processed
        type_derivation_function_t *func = (type_derivation_function_t*) malloc(sizeof(type_derivation_function_t));
        func->type = DERIVATION_FUNCTION;
        mark_current(ctx->ptr);
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_L_P);
        parse_param_list(ctx, &(func->params));
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P);
    } else {
        return false;
    }

    if (has_ptr_decl) {
        ARRAY_LIST_APPEND(*derivations, ((type_derivation_t*) ptr_derivation), type_derivation_t*);
    }

    return true;
}

bool parse_abst_declarator(context_t *ctx, ast_node_t *parent, ast_typename_t *type_in, ast_typename_t **dest) {
    mark_current(ctx->ptr);
    type_derivation_stack_t derivations;
    ARRAY_LIST_INIT(const type_derivation_t*, derivations)
    if (!parse_abst_declarator_internal(ctx, &derivations)) {
        return false;
    }

    while (!ARRAY_LIST_EMPTY_INLINE(derivations)) {
        const type_derivation_t *derivation;
        ARRAY_LIST_REMOVE_TAIL(derivations, derivation)
        switch (derivation->type) {
        case DERIVATION_FUNCTION:
            const type_derivation_function_t *fn_derv = (type_derivation_function_t*) derivation;
            ast_typename_funct_t *fn_node = (ast_typename_funct_t*) malloc(sizeof(ast_typename_funct_t));
            fn_node->node_type = AST_TYPE_FUNCT;
            type_in->parent = (ast_node_t*) fn_node;
            fn_node->return_type = type_in;
            fn_node->immutable = false;
            memcpy(&(fn_node->param_type), &(fn_derv->params), sizeof(param_list_t));
            type_in = (ast_typename_t*) fn_node;
            break;
        case DERIVATION_POINTER:
            const type_derivation_pointer_t *ptr_derv = (type_derivation_pointer_t*) derivation;
            ARRAY_LIST_TRAVERSE(ptr_derv->constant, bool, constant, i, {
                ast_typename_ptr_t *ptr_node = (ast_typename_ptr_t*) malloc(sizeof(ast_typename_ptr_t));
                ptr_node->node_type = AST_TYPE_PTR;
                type_in->parent = (ast_node_t*) ptr_node;
                ptr_node->immutable = constant;
                ptr_node->underlying_type = type_in;
                type_in = (ast_typename_t*) ptr_node;
            })
            break;
        default:
            fatal_on_token(peek_current(ctx->ptr), "Unrecognized type derivation: %d\n", derivation->type);
            return false;
        }
    }

    type_in->parent = parent;
    *dest = (ast_typename_t*) type_in;
    return true;
}

void type_spec_to_ast_typename_node(uint_t spec, ast_node_t *parent, ast_typename_t **dest) {
    ast_typename_prim_t *new_node = (ast_typename_prim_t*) malloc(sizeof(ast_typename_prim_t));
    new_node->node_type = AST_TYPE_PRIM;
    new_node->parent = parent;
    new_node->immutable = (spec & DECL_SPEC_CONST) != 0;
    if (spec & DECL_SPEC_INT8_T) {
        new_node->type_name = ETYPE_INT_8;
    } else if (spec & DECL_SPEC_UINT8_T) {
        new_node->type_name = ETYPE_UINT_8;
    } else if (spec & DECL_SPEC_VOID) {
        new_node->type_name = ETYPE_VOID;
    }

    *dest = (ast_typename_t*) new_node;
}

// param-decl		:= decl-spec declarator | type-name

bool parse_declarator(context_t *ctx, ast_node_t *parent, ast_typename_t *decl_type, ast_decl_t **dest);

extern symbol_t UNSPECIFIED_SYMBOL;

bool parse_param_decl(context_t *ctx, ast_decl_direct_param_t **dest) {
    debug(" ==> Parsing: param-decl\n");
    uint_t decl_spec = 0;
    ast_decl_direct_param_t *param_decl = (ast_decl_direct_param_t*) malloc(sizeof(ast_decl_direct_param_t));
    param_decl->node_type = AST_DECL_DRCT_PARAM;
    param_decl->parent = NULL;
    if (parse_typename(ctx, (ast_node_t*) param_decl, (ast_typename_t**) &(param_decl->var_type))) {
        param_decl->var_name = &UNSPECIFIED_SYMBOL;
    } else if (parse_decl_specifier(ctx, &decl_spec)) {
        ast_typename_t *decl_type;
        type_spec_to_ast_typename_node(decl_spec, (ast_node_t*) param_decl, (ast_typename_t**) &decl_type);
        ast_decl_direct_variable_t *param_var_decl;
        parse_declarator(ctx, (ast_node_t*) param_decl, decl_type, (ast_decl_t**) &param_var_decl);
        if (param_var_decl->var_name->type != SYM_VARIABLE) {
            error_on_token(peek_current(ctx->ptr), "Parameters must be variables!\n");
            return false;
        }

        param_decl->var_name = param_var_decl->var_name;
        param_decl->var_type = param_var_decl->var_type;
    } else {
        return false;
    }

    *dest = param_decl;
    return true;
}

// param-list		:= param-decl | param-list , param-decl

bool parse_param_list(context_t *ctx, param_list_t *dest) {
    debug(" ==> Parsing: param-list\n");
    while (peek_current(ctx->ptr)->type != TOKEN_PUNCT_R_P) {
        ast_decl_direct_param_t *param_decl;
        if (parse_param_decl(ctx, &param_decl)) {
            ARRAY_LIST_APPEND(*dest, param_decl, ast_decl_direct_param_t);
        } else {
            return false;
        }

        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_COMMA);
    }

    return true;
}

// type-name		:= decl-spec abst-declarator?

// A good example: char (*(*)(float)) (void);

bool parse_typename(context_t *ctx, ast_node_t *parent, ast_typename_t **dest) {
    debug(" ==> Parsing: type-name\n");
    uint_t decl_spec;
    if (!parse_decl_specifier(ctx, &decl_spec)) {
        return false;
    }

    ast_typename_t *decl_type;
    type_spec_to_ast_typename_node(decl_spec, NULL, &decl_type);
    *dest = decl_type;
    parse_abst_declarator(ctx, parent, decl_type, dest);
    return true;
}

// declarator		:= pointer? drct-declarator
// drct-declarator	:= identifier
// 					| ( declarator )
// 					| drct-declarator ( param-list? )

// We shall follow the same scheme we've applied to parse abst-declarators

bool parse_declarator_internal(context_t *ctx, type_derivation_stack_t *derivations, const token_t **ident) {
    type_derivation_pointer_t *ptr_derivation;
    bool has_ptr_decl;
    has_ptr_decl = parse_pointer_decl(ctx, &ptr_derivation);
    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        // ( A ) part
        skip_current(ctx->ptr);
        if (!parse_declarator_internal(ctx, derivations, ident)) {
            if (has_ptr_decl) {
                return true;
            } else {
                return_marked(ctx->ptr);
                return false;
            }
        }

        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P);
    } else {
        // A := P and an identifier follows
        ARRAY_LIST_APPEND(*derivations, ((type_derivation_t*) ptr_derivation), type_derivation_t*);
        if (peek_current(ctx->ptr)->type == TOKEN_IDENTIFIER) {
            *ident = read_current(ctx->ptr);
            return true;
        } else {
            return false;
        }
    }

    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        // A := P? ( A ) ( Q ) and we are parsing ( Q ) since P? ( A ) has already been processed
        type_derivation_function_t *func = (type_derivation_function_t*) malloc(sizeof(type_derivation_function_t));
        func->type = DERIVATION_FUNCTION;
        mark_current(ctx->ptr);
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_L_P);
        parse_param_list(ctx, &(func->params));
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P);
    } else {
        return false;
    }

    if (has_ptr_decl) {
        ARRAY_LIST_APPEND(*derivations, ((type_derivation_t*) ptr_derivation), type_derivation_t*);
    }

    return true;
}

bool parse_declarator(context_t *ctx, ast_node_t *parent, ast_typename_t *type_in, ast_decl_t **dest) {
    mark_current(ctx->ptr);
    type_derivation_stack_t derivations;
    const token_t *ident_token;
    if (!parse_declarator_internal(ctx, &derivations, &ident_token)) {
        return false;
    }

    while (!ARRAY_LIST_EMPTY_INLINE(derivations)) {
        const type_derivation_t *derivation;
        ARRAY_LIST_REMOVE_TAIL(derivations, derivation)
        switch (derivation->type) {
        case DERIVATION_FUNCTION:
            const type_derivation_function_t *fn_derv = (type_derivation_function_t*) derivation;
            ast_typename_funct_t *fn_node = (ast_typename_funct_t*) malloc(sizeof(ast_typename_funct_t));
            fn_node->node_type = AST_TYPE_FUNCT;
            type_in->parent = (ast_node_t*) fn_node;
            fn_node->return_type = type_in;
            fn_node->immutable = false;
            memcpy(&(fn_node->param_type), &(fn_derv->params), sizeof(param_list_t));
            type_in = (ast_typename_t*) fn_node;
            break;
        case DERIVATION_POINTER:
            const type_derivation_pointer_t *ptr_derv = (type_derivation_pointer_t*) derivation;
            ARRAY_LIST_TRAVERSE(ptr_derv->constant, bool, constant, i, {
                ast_typename_ptr_t *ptr_node = (ast_typename_ptr_t*) malloc(sizeof(ast_typename_ptr_t));
                ptr_node->node_type = AST_TYPE_PTR;
                type_in->parent = (ast_node_t*) ptr_node;
                ptr_node->immutable = constant;
                ptr_node->underlying_type = type_in;
                type_in = (ast_typename_t*) ptr_derv;
            })
            break;
        default:
            fatal_on_token(peek_current(ctx->ptr), "Unrecognized type derivation: %d\n", derivation->type);
            return false;
        }
    }

    type_in->parent = parent;
    return true;
}

// *********** Main Procedure ***********

// compile-unit	:= extern-decl | compile-unit
// extern-decl	:= func-def | decl
// func-def		:= decl-spec declarator decl-list? comp-stmt
// decl-list	:= decl | decl-list decl

void parse(context_t *ctx) {
    ast_expr_t *expr;
    parse_expr(ctx, NULL, &expr);
    dump_ast((ast_node_t*) expr, NULL, "Root");
}
