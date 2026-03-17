#include "parser.h"

#include "context.h"
#include "ast.h"

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

extern uint_t error_cnt, warning_cnt;

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
    return inbounds(ptr) ? ptr->base[ptr->cur_pos++] : NULL;
}

void unread_prev(read_head_t *ptr) {
    if (ptr->cur_pos > 0) {
        ptr->cur_pos--;
    }
}

void skip_current(read_head_t *ptr) {
    // printf(peek_current(ptr));
    if (inbounds(ptr)) (ptr->cur_pos)++;
}

void verify_and_skip_current(read_head_t *ptr, token_type_t type) {
    // printf(peek_current(ptr));
    if (peek_current(ptr)->type != type) {
        error_on_token(peek_current(ptr), "Expected token %d but got %d!\n", type, peek_current(ptr)->type);
    }

    if (inbounds(ptr)) (ptr->cur_pos)++;
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
        *dest = (ast_expr_t*) const_node;
        return true;
    case TOKEN_IDENTIFIER:
        ast_expr_symbol_t *sym_node = (ast_expr_symbol_t *) malloc(sizeof(ast_expr_symbol_t));
        sym_node->node_type = AST_EXPR_CONST;
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
        }
    case TOKEN_PUNCT_L_P:
        // We've skipped '('
        bool is_expr = parse_expr(ctx, parent, dest);
        skip_current(ctx->ptr);
        return is_expr;
    default:

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
    ast_expr_t *left_hand;
    if (!parse_primary_expr(ctx, NULL, &left_hand)) {
        return false;
    }

    const token_t *postfix_start = read_current(ctx->ptr);
    switch (postfix_start->type) {
    case TOKEN_PUNCT_L_SP:
        // Array subscript
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
    case TOKEN_PUNCT_L_CP:
        // Function call
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
        ast_expr_unary_t *getinc_node = (ast_expr_unary_t*) malloc(sizeof(ast_expr_unary_t));
        getinc_node->node_type = AST_EXPR_UNARY;
        getinc_node->parent = parent;
        getinc_node->op = postfix_start->type == TOKEN_PUNCT_INC ? UOP_GETINC : UOP_GETDEC;
        getinc_node->constant = false;
        getinc_node->lvalue = false;
        getinc_node->opnd = left_hand;
        if (!left_hand->lvalue) {
            error_on_token(postfix_start, "Operand of ++ or -- must be lvalue!");
            return true;
        }

        left_hand->parent = (ast_node_t*) getinc_node;
        *dest = (ast_expr_t*) getinc_node;
        return true;
    default:
        // Unexpected
        return false;
    }
}

// unary-expr   := postfix-expr
// 					| ++ unary-expr
// 					| -- unary-expr
// 					| unary-op unary-expr
// 					| sizeof unary-expr
// 					| sizeof ( type-name )
// unary-op		:= & * + - ~ !

bool parse_typename(context_t *ctx, ast_node_t *parent, ast_expr_t **dest);

bool parse_unary_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    const token_t *prefix = read_current(ctx->ptr);
    switch (prefix->type) {
    case TOKEN_PUNCT_INC:
    case TOKEN_PUNCT_DEC:
        // Increment and get
        // Decrement and get
        ast_expr_unary_t *getinc_node = (ast_expr_unary_t*) malloc(sizeof(ast_expr_unary_t));
        getinc_node->node_type = AST_EXPR_UNARY;
        getinc_node->parent = parent;
        getinc_node->op = prefix->type == TOKEN_PUNCT_INC ? UOP_INCGET : UOP_DECGET;
        getinc_node->constant = false;
        getinc_node->lvalue = false;
        parse_unary_expr(ctx, (ast_node_t*) getinc_node, (ast_expr_t**) &(getinc_node->opnd));
        if (!getinc_node->opnd->lvalue) {
            error_on_token(prefix, "Operand of ++ or -- must be lvalue!");
            return true;
        }

        *dest = (ast_expr_t*) getinc_node;
        return true;
    case TOKEN_PUNCT_AND:
    case TOKEN_PUNCT_STAR:
    case TOKEN_PUNCT_PLUS:
    case TOKEN_PUNCT_MINUS:
    case TOKEN_PUNCT_NEG:
    case TOKEN_PUNCT_NOT:
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

        parse_unary_expr(ctx, (ast_node_t*) unary_node, (ast_expr_t**) &(unary_node->opnd));
        unary_node->constant = unary_node->opnd->constant;
        *dest = (ast_expr_t*) unary_node;
        return true;
        // Unary ops
    case TOKEN_KW_SIZEOF:
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
            parse_typename(ctx, (ast_node_t*) sizeof_node, (ast_expr_t**) &(sizeof_node->sizeof_type));
            verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_R_P); // Skip ')'
            *dest = (ast_expr_t*) sizeof_node;
            return true;
        }
    default:
        unread_prev(ctx->ptr);
        return parse_postfix_expr(ctx, parent, dest);
    }
}

// cast-expr	:= unary-expr
// 					| \( type-name \) cast-expr

bool parse_cast_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    // FIXME
    if (peek_current(ctx->ptr)->type == TOKEN_PUNCT_L_P) {
        ast_expr_cast_t *cast_node = (ast_expr_cast_t*) malloc(sizeof(ast_expr_cast_t));
        cast_node->lvalue = false;
        cast_node->node_type = AST_EXPR_CAST;
        cast_node->parent = parent;
        verify_and_skip_current(ctx->ptr, TOKEN_PUNCT_L_P); // Skip '('
        if (!parse_typename(ctx, (ast_node_t*) cast_node, (ast_expr_t**) &(cast_node->cast_to))) {
            // It is prim-expr '(E)' instead of '(T) E'
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
    return parse_binary_expr(ctx, parent, dest, equality_op_token_to_op, parse_relation_expr);
}

// and-expr		:= equality-expr
// 					| and-expr & equality-expr

binary_op_t and_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_AND) ? BOP_AND : BOP_UNRECOGNIZED;
}

bool parse_and_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return parse_binary_expr(ctx, parent, dest, and_op_token_to_op, parse_equality_expr);
}

// xor-expr		:= and-expr
// 					| xor-expr ^ and-expr

binary_op_t xor_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_XOR) ? BOP_XOR : BOP_UNRECOGNIZED;
}

bool parse_xor_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return parse_binary_expr(ctx, parent, dest, xor_op_token_to_op, parse_and_expr);
}

// or-expr			:= xor-expr
// 					| or-expr \| xor-expr

binary_op_t or_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_OR) ? BOP_OR : BOP_UNRECOGNIZED;
}

bool parse_or_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return parse_binary_expr(ctx, parent, dest, or_op_token_to_op, parse_xor_expr);
}

// land-expr		:= or-expr
// 					| land-expr && or-expr

binary_op_t land_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_LAND) ? BOP_LAND : BOP_UNRECOGNIZED;
}

bool parse_land_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return parse_binary_expr(ctx, parent, dest, land_op_token_to_op, parse_or_expr);
}

// lor-expr		:= land-expr
// 					| lor-expr \|\| land-expr

binary_op_t lor_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_LAND) ? BOP_LAND : BOP_UNRECOGNIZED;
}

bool parse_lor_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
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
    ast_expr_t *left_opnd;
    // Apparently, we cannot call parse_multiplicative_expr() unconditionally
    // We have to push something forward beforehand at least.
    // Also, we note that mult-expr := cast-expr (mult-op cast-expr)*, which leads to following algorithm
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
        error_on_token(assign_op_token, "Unexpected token in assign-expr: %d\n", assign_op_token->type);
        return false;
    }

    ast_expr_assign_t *assign_node = (ast_expr_assign_t*) malloc(sizeof(ast_expr_assign_t));
    assign_node->node_type = AST_EXPR_ASSIGN;
    assign_node->parent = parent;
    assign_node->constant = false;
    assign_node->op = assign_op;
    assign_node->dest = assign_dest;
    parse_assign_expr(ctx, (ast_node_t*) assign_node, (ast_expr_t**) &(assign_node->src));
    assign_node->lvalue = assign_node->src->lvalue;    // XXX
    return true;
}

// expr			:= assign-expr | expr , assign-expr

binary_op_t comma_op_token_to_op(const token_t *op_token) {
    return (op_token->type == TOKEN_PUNCT_COMMA) ? BOP_COMMA : BOP_UNRECOGNIZED;
}

bool parse_expr(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return parse_binary_expr(ctx, parent, dest, comma_op_token_to_op, parse_assign_expr);
}

// *********** Type Names ***********

bool parse_typename(context_t *ctx, ast_node_t *parent, ast_expr_t **dest) {
    return false;    // Dummy
}

// *********** Main Procedure ***********

// compile-unit	:= extern-decl | compile-unit
// extern-decl	:= func-def | decl
// func-def		:= decl-spec declarator decl-list? comp-stmt
// decl-list	:= decl | decl-list decl

void parse(context_t *ctx, bool verbose) {
    ast_expr_t *expr;
    parse_expr(ctx, NULL, &expr);
}
