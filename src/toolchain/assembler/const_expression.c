#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "const_expression.h"
#include "parser.h"
#include "stack.h"
#include "map.h"

bool at_expr_delim(read_head *ptr) {
    if (!inbounds(ptr)) return true;
    char_t c = *peek_current(ptr);
    return c == '\r' || c == '\n' || c == ',' || c == '%' || c == ')';
}

bool at_operator(read_head *ptr) {
    if (!inbounds(ptr)) return false;
    char_t c = *peek_current(ptr);
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '^';
}

int32_t eval_next_atomic_token(parse_ctx *ctx, read_head *ptr) {
    str numeric_token = read_current(ptr);
    int32_t num;
    if (numeric_token[0] == '0') {
        // We guarantee that tokens are not empty - and thus has subscription 1
        switch (tolower(numeric_token[1])) {
        case 'x':
            return strtol(numeric_token + 2, NULL, 16);
        case 'b':
            num = 0;
            for (int i = 2; i < strlen(numeric_token); i++) {
                num = (num << 1) | ((numeric_token[i] == '0') ? 0 : 1);
            }

            return num;
        default:
            return strtol(numeric_token, NULL, 8);
        }
    } else if (isdigit((uchar_t) numeric_token[0])) {
        return strtol(numeric_token, NULL, 10);
    } else {
        if ((num = get_constant(ctx, numeric_token)) != UNDEFINED_CONSTANT) {
            return num;
        } else if ((num = get_pointer(ctx, numeric_token)) != UNDEFINED_CONSTANT) {
            return num;
        } else if ((num = get_label(ctx, numeric_token)) != UNDEFINED_CONSTANT) {
            return num;
        } else {
            error("Undefined constant: %s\n", numeric_token);
            return -1;
        }
    }
}

// Using int32_t since it is capable to hold any value between -4096 - 4095
int32_t eval_next_number(parse_ctx *ctx, read_head *ptr) {
    assert_inbounds(ptr);
    int32_t sign = 1;
    str token = peek_current(ptr);
    if (*token == '-') {
        sign = -1;
        skip_current(ptr);
    } else if (*token == '+') {
        skip_current(ptr);
    } else if (*token == '(') {
        int_t val = eval_const_expr(ctx, ptr);
        skip_current(ptr);  // Skip ')'
        return val;
    }

    return sign * eval_next_atomic_token(ctx, ptr);
}

static int32_t op_number[] = {
    ['%'] = 0,
    ['/'] = 1,
    ['*'] = 2,
    ['-'] = 3,
    ['+'] = 4,
    ['&'] = 5,
    ['^'] = 6,
    ['|'] = 7,
    ['#'] = 8
};

static int32_t precedence_table[sizeof(op_number)][sizeof(op_number)] = {
    {   0,  0,  0,  1,  1,  1,  1,  1,  1   },
    {   0,  0,  0,  1,  1,  1,  1,  1,  1   },
    {   0,  0,  0,  1,  1,  1,  1,  1,  1   },
    {  -1, -1, -1,  0,  0,  1,  1,  1,  1   },
    {  -1, -1, -1,  0,  0,  1,  1,  1,  1   },
    {  -1, -1, -1, -1, -1,  0,  1,  1,  1   },
    {  -1, -1, -1, -1, -1, -1,  0,  1,  1   },
    {  -1, -1, -1, -1, -1, -1, -1,  0,  1   },
    {  -1, -1, -1, -1, -1, -1, -1, -1,  0   },
};

int32_t cmp_precedence(char_t op1, char_t op2) {
    return precedence_table[op_number[(uchar_t) op1]][op_number[(uchar_t) op2]];
}

int32_t apply_op(char_t op, int32_t x, int32_t y) {
    switch (op) {
    case '+':
        return x + y;
    case '-':
        return x - y;
    case '*':
        return x * y;
    case '/':
        return x / y;
    case '&':
        return x & y;
    case '|':
        return x | y;
    case '^':
        return x ^ y;
    default:
        error("Unrecognized operator: %c\n", op);
        return -1;
    }
}

// One pass
int32_t eval_const_expr(parse_ctx *ctx, read_head *ptr) {
    STACK_TYPE(char_t) deferred_ops;
    STACK_TYPE(uint_t) deferred_opnds;
    STACK_INIT(char_t, deferred_ops)
    STACK_INIT(int32_t, deferred_opnds)
    STACK_PUSH(deferred_ops, '#', char_t)
    STACK_PUSH(deferred_opnds, eval_next_number(ctx, ptr), int32_t)
    char_t op, prev_op;
    while (!at_expr_delim(ptr)) {
        assert_inbounds(ptr);
        op = *read_current(ptr);
        STACK_PEEK(deferred_ops, prev_op)
        if (cmp_precedence(op, prev_op) > 0) {
            STACK_PUSH(deferred_ops, op, char_t)
            STACK_PUSH(deferred_opnds, eval_next_number(ctx, ptr), int32_t)
        } else {
            do {
                uint_t opnd_r, opnd_l;
                STACK_POP(deferred_ops, prev_op)
                STACK_POP(deferred_opnds, opnd_r)
                STACK_POP(deferred_opnds, opnd_l)
                STACK_PUSH(deferred_opnds, apply_op(prev_op, opnd_l, opnd_r), int32_t)
                STACK_PEEK(deferred_ops, prev_op)
            } while (cmp_precedence(op, prev_op) <= 0);
            STACK_PUSH(deferred_ops, op, char_t)
            STACK_PUSH(deferred_opnds, eval_next_number(ctx, ptr), int32_t)
        }
    }

    STACK_PEEK(deferred_ops, op);
    while (op != '#') {
        uint_t opnd_r, opnd_l;
        STACK_POP(deferred_ops, op)
        STACK_POP(deferred_opnds, opnd_r)
        STACK_POP(deferred_opnds, opnd_l)
        STACK_PUSH(deferred_opnds, apply_op(op, opnd_l, opnd_r), int32_t)
        STACK_PEEK(deferred_ops, op)
    }

    uint_t result;
    STACK_POP(deferred_opnds, result)
    STACK_FREE(deferred_ops)
    STACK_FREE(deferred_opnds)
    return result;
}
