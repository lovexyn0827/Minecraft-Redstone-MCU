#include <stdint.h>

#include "parser.h"
#include "stack.h"
#include "map.h"

bool at_expr_delim(read_head *ptr) {
    if (inbounds(ptr)) return true;
    uint8_t c = *peek_current(ptr);
    return c == '\r' || c == '\n' || c == ',';
}

bool at_operator(read_head *ptr) {
    if (!inbounds(ptr)) return false;
    uint8_t c = *peek_current(ptr);
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '|' || c == '^';
}

// Using int32_t since it is capable to hold any value between -4096 - 4095
int32_t eval_next_atomic_token(read_head *ptr) {
    assert_inbounds(ptr);
    while (inbounds(ptr)) {
        uint8_t c = peek_current(ptr);

    }
}

static int32_t op_number[128] = { 0 };
static int32_t precedence_table[sizeof(op_number)][sizeof(op_number)] = {
    {   0,  0,  0,  1,  1,  1,  1,  1, -1, -1,  1   },
    {   0,  0,  0,  1,  1,  1,  1,  1, -1, -1,  1   },
    {   0,  0,  0,  1,  1,  1,  1,  1, -1, -1,  1   },
    {  -1, -1, -1,  0,  0,  1,  1,  1, -1, -1,  1   },
    {  -1, -1, -1,  0,  0,  1,  1,  1, -1, -1,  1   },
    {  -1, -1, -1, -1, -1,  0,  1,  1, -1, -1,  1   },
    {  -1, -1, -1, -1, -1, -1,  0,  1, -1, -1,  1   },
    {  -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,  1   },
    {   1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1   },
    {   1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1   },
    {  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0   },
};

int32_t cmp_precedence(uint8_t op1, uint8_t op2) {
    op_number['&'] = 0;
    op_number['/'] = 1;
    op_number['*'] = 2;
    op_number['-'] = 3;
    op_number['+'] = 4;
    op_number['&'] = 5;
    op_number['^'] = 6;
    op_number['|'] = 7;
    op_number['('] = 8;
    op_number[')'] = 9;
    op_number['#'] = 10;

    return precedence_table[op_number[op1]][op_number[op2]];
}

int32_t apply_op(uint8_t op, int32_t opnd1, int32_t opnd2) {
}

// One pass
int32_t eval_const_expr(parse_ctx *ctx, read_head *ptr) {
    STACK_TYPE(uint8_t) deferred_ops;
    STACK_TYPE(uint32_t) deferred_opnds;
    STACK_INIT(uint8_t, deferred_ops)
    STACK_INIT(int32_t, deferred_opnds)
    STACK_PUSH(deferred_ops, '#', uint8_t)
    STACK_PUSH(deferred_opnds, eval_next_atomic_token(ptr), int32_t)
    uint8_t op, prev_op;
    while (!at_expr_delim(ptr)) {
        assert_inbounds(ptr);
        op = *peek_current(ptr);
        STACK_PEEK(deferred_ops, prev_op)
        if (cmp_precedence(op, prev_op) > 0) {
            STACK_PUSH(deferred_ops, op, uint8_t)
            STACK_PUSH(deferred_opnds, eval_next_atomic_token(ptr), int32_t)
        } else {
            STACK_POP(deferred_ops, prev_op)
            STACK_POP(deferred_opnds, int32_t opnd_r)
            STACK_POP(deferred_opnds, int32_t opnd_l)
            STACK_PUSH(deferred_opnds, apply_op(prev_op, opnd_l, opnd_r), int32_t)
        }
    }

    STACK_PEEK(deferred_ops, op);
    while (op != '#') {
        STACK_POP(deferred_ops, op)
        STACK_POP(deferred_opnds, int32_t opnd_r)
        STACK_POP(deferred_opnds, int32_t opnd_l)
        STACK_PUSH(deferred_opnds, apply_op(op, opnd_l, opnd_r), int32_t)
        STACK_PEEK(deferred_ops, op)
    }

    STACK_POP(deferred_opnds, int32_t result)
    STACK_FREE(deferred_ops)
    STACK_FREE(deferred_opnds)
    return result;
}
