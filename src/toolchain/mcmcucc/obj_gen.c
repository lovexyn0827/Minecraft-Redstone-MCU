#include "obj_gen.h"

void build_unary_expr_evaluator(const ast_expr_unary_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_binary_expr_evaluator(const ast_expr_binary_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_cond_expr_evaluator(const ast_expr_cond_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_assign_expr_evaluator(const ast_expr_assign_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_call_expr_evaluator(const ast_expr_call_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_symbol_expr_evaluator(const ast_expr_symbol_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_expr_evaluator(const ast_expr_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
    switch (expr->node_type) {
    case AST_EXPR_UNARY:
        build_unary_expr_evaluator((const ast_expr_unary_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_BINARY:
        build_binary_expr_evaluator((const ast_expr_binary_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_COND:
        build_cond_expr_evaluator((const ast_expr_cond_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_CAST:
        // Requires no insn
        break;
    case AST_EXPR_EXPRSZ:
        // Swept-off by constant folding
        break;
    case AST_EXPR_TYPESZ:
        // Swept-off by constant folding
        break;
    case AST_EXPR_ASSIGN:
        build_assign_expr_evaluator((const ast_expr_assign_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_CALL:
        build_call_expr_evaluator((const ast_expr_call_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_CONST:
        // Automatically handled in involved insns
        break;
    case AST_EXPR_SYMBOL:
        build_symbol_expr_evaluator((const ast_expr_symbol_t*) expr, val_dest, asm_dest);
        break;
    default:
        fatal("Expected expr nodes!\n");
    }
}

void generate_asm(context_t *ctx, FILE *asm_dest) {
}
