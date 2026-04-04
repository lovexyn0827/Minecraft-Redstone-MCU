#include "opt_passes.h"

// *********** Constant Folding ***********

ast_node_t *new_const_node(ast_expr_t *original, uint_t val) {
    ast_expr_constant_t *const_node = (ast_expr_constant_t*) malloc(sizeof(ast_expr_constant_t));
    const_node->node_type = AST_EXPR_CONST;
    const_node->lvalue = false;
    const_node->parent = original->parent;
    const_node->constant = true;
    const_node->value = val & 0xFF;
    const_node->address = (obj_addr_t*) malloc(sizeof(obj_addr_t));
    const_node->address->type = OBJ_ADDR_IMM;
    const_node->address->addr = val;
    return (ast_node_t*) const_node;
}

ast_node_t *eval_constexpr_unary(ast_expr_unary_t *node) {
    if (!node->constant) return NULL;
    uint_t opnd = ((ast_expr_constant_t*) node->opnd)->value;
    uint_t val;
    switch (node->op) {
    case UOP_BITWISE_NOT:
        val = ~opnd;
        break;
    case UOP_LOGICAL_NOT:
        val = !opnd;
        break;
    case UOP_NEGATIVATE:
        val = -opnd;
        break;
    case UOP_POSITIVE:
        val = opnd;
        break;
    default:
        return NULL;
    }

    return new_const_node((ast_expr_t*) node, val);
}

ast_node_t *eval_constexpr_binary(ast_expr_binary_t *node) {
    if (!node->constant) return NULL;
    uint_t lopnd = ((ast_expr_constant_t*) node->left_opnd)->value;
    uint_t ropnd = ((ast_expr_constant_t*) node->right_opnd)->value;
    uint_t val;
    switch (node->op) {
    case BOP_ADD:
        val = lopnd + ropnd;
        break;
    case BOP_SUB:
        val = lopnd - ropnd;
        break;
    case BOP_MUL:
        val = lopnd * ropnd;
        break;
    case BOP_DIV:
        val = lopnd / ropnd;
        break;
    case BOP_MOD:
        val = lopnd % ropnd;
        break;
    case BOP_AND:
        val = lopnd & ropnd;
        break;
    case BOP_OR:
        val = lopnd | ropnd;
        break;
    case BOP_LAND:
        val = lopnd && ropnd;
        break;
    case BOP_LOR:
        val = lopnd || ropnd;
        break;
    case BOP_XOR:
        val = lopnd ^ ropnd;
        break;
    case BOP_COMMA:
        val = ropnd;
        break;
    case BOP_EQ:
        val = lopnd == ropnd;
        break;
    case BOP_LE:
        val = lopnd <= ropnd;
        break;
    case BOP_GE:
        val = lopnd >= ropnd;
        break;
    case BOP_LT:
        val = lopnd < ropnd;
        break;
    case BOP_GT:
        val = lopnd > ropnd;
        break;
    case BOP_NE:
        val = lopnd != ropnd;
        break;
    case BOP_SHL:
        val = lopnd << ropnd;
        break;
    case BOP_SHR:
        val = lopnd >> ropnd;
        break;
    default:
        return NULL;
    }

    return new_const_node((ast_expr_t*) node, val);
}

ast_node_t *eval_constexpr_cond(ast_expr_cond_t *node) {
    if (!node->constant) return NULL;
    uint_t val;
    if (((ast_expr_constant_t*) node->cond)->value) {
        val = ((ast_expr_constant_t*) node->if_true)->value;
    } else {
        val = ((ast_expr_constant_t*) node->if_false)->value;
    }

    return new_const_node((ast_expr_t*) node, val);
}

ast_node_t *eval_constexpr_cast(ast_expr_cast_t *node) {
    if (!node->constant) return NULL;
    return new_const_node((ast_expr_t*) node, ((ast_expr_constant_t*) node->opnd)->value);
}

ast_node_t *eval_constexpr_sizeof_expr(ast_expr_sizeof_expr_t *node) {
    return new_const_node((ast_expr_t*) node, 1);
}

ast_node_t *eval_constexpr_sizeof_type(ast_expr_sizeof_type_t *node) {
    return new_const_node((ast_expr_t*) node, 1);
}

ast_node_t *eval_constexpr(ast_node_t *node) {
    switch (node->node_type) {
    case AST_EXPR_UNARY:
        return eval_constexpr_unary((ast_expr_unary_t*) node);
    case AST_EXPR_BINARY:
        return eval_constexpr_binary((ast_expr_binary_t*) node);
    case AST_EXPR_COND:
        return eval_constexpr_cond((ast_expr_cond_t*) node);
    case AST_EXPR_CAST:
        return eval_constexpr_cast((ast_expr_cast_t*) node);
    case AST_EXPR_EXPRSZ:
        return eval_constexpr_sizeof_expr((ast_expr_sizeof_expr_t*) node);
    case AST_EXPR_TYPESZ:
        return eval_constexpr_sizeof_type((ast_expr_sizeof_type_t*) node);
    default:
        return NULL;
    }
}

void fold_constants(context_t *ctx) {
    for_each_node((ast_node_t*) &(ctx->ast.root), NULL, eval_constexpr);
    debug("##### AFTER Constant Folding: \n");
    dump_ast((ast_node_t*) &(ctx->ast.root), NULL, "");
}

// *********** Main Procedure ***********

void optimize_ast(context_t *ctx) {
    fold_constants(ctx);
}
