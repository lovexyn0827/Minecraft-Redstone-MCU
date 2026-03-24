#include "ast.h"

#include "common.h"

#include <stdio.h>

void dump_ast(const ast_node_t *node, const ast_node_t *parent, str field) {
    debug("%08x --> %08x: %s\n", parent, node, field);
    switch (node->node_type) {
    case AST_ROOT:
        debug("%08x: ROOT_%08x\n", node, node);
        break;
    case AST_TYPENAME:
        break;
    case AST_TYPE_PRIM:
        const ast_typename_prim_t *prim_type_node = (const ast_typename_prim_t*) node;
        debug("%08x: PRIM_TYPE_%08x_%d\n", node, node, prim_type_node->type_name);
        break;
    case AST_TYPE_PTR:
        const ast_typename_ptr_t *ptr_type_node = (const ast_typename_ptr_t*) node;
        if (ptr_type_node->immutable) {
            debug("%08x: CONSTPTR_%08x\n", node, node);
        } else {
            debug("%08x: MUTPTR_%08x\n", node, node);
        }

        dump_ast((const ast_node_t*) ptr_type_node->underlying_type, node, "Underlying");
        break;
    case AST_TYPE_FUNCT:
        const ast_typename_funct_t *fn_type_node = (const ast_typename_funct_t*) node;
        debug("%08x: FN_TYPE_%08x\n", node, node);
        dump_ast((const ast_node_t*) fn_type_node->return_type, node, "Ret");
        ARRAY_LIST_TRAVERSE(fn_type_node->param_type, const ast_decl_direct_param_t*, param, param_idx, {
            char_t arg_idx_buf[16];
            sprintf(arg_idx_buf, "Param_%d", param_idx);
            dump_ast((const ast_node_t*) param, node, arg_idx_buf);
        })
        break;
    case AST_STMT:
        break;
    case AST_STMT_DECL:
        break;
    case AST_STMT_EXPR:
        break;
    case AST_STMT_IF:
        break;
    case AST_STMT_IFELSE:
        break;
    case AST_STMT_FOR:
        break;
    case AST_STMT_FORDECL:
        break;
    case AST_STMT_WHILE:
        break;
    case AST_STMT_DO:
        break;
    case AST_STMT_SWITCH:
        break;
    case AST_STMT_GOTO:
        break;
    case AST_STMT_BREAK:
        break;
    case AST_STMT_CONTINUE:
        break;
    case AST_STMT_RETURN:
        break;
    case AST_STMT_COMPOUND:
        break;
    case AST_STMT_CASE:
        break;
    case AST_STMT_DEFAULT:
        break;
    case AST_STMT_LABELED:
        break;
    case AST_DECL:
        break;
    case AST_DECL_DRCT_FN:
        break;
    case AST_DECL_DRCT_PARAM:
        break;
    case AST_DECL_DRCT_VAR:
        break;
    case AST_DECL_DRCT_ARRAY:
        break;
    case AST_EXPR:
        break;
    case AST_EXPR_CALL:
        const ast_expr_call_t *call_node = (const ast_expr_call_t*) node;
        debug("%08x: EXPR_CALL_%08x\n", node, node);
        dump_ast((const ast_node_t*) call_node->function_addr, node, "Func");
        ARRAY_LIST_TRAVERSE(call_node->arguments, const ast_expr_t*, arg, arg_idx, {
            char_t arg_idx_buf[16];
            sprintf(arg_idx_buf, "Arg_%d", arg_idx);
            dump_ast((const ast_node_t*) arg, node, arg_idx_buf);
        })
        break;
    case AST_EXPR_SYMBOL:
        break;
    case AST_EXPR_CONST:
        const ast_expr_constant_t *const_node = (const ast_expr_constant_t*) node;
        debug("%08x: EXPR_CONST_%08x_%d\n", node, node, const_node->value);
        break;
    case AST_EXPR_UNARY:
        const ast_expr_unary_t *unary_node = (const ast_expr_unary_t*) node;
        debug("%08x: EXPR_UNARY_%08x_%d\n", node, node, unary_node->op);
        dump_ast((const ast_node_t*) unary_node->opnd, node, "Opnd");
        break;
    case AST_EXPR_CAST:
        const ast_expr_cast_t *cast_node = (const ast_expr_cast_t*) node;
        debug("%08x: EXPR_CAST_%08x\n", node, node);
        dump_ast((const ast_node_t*) cast_node->cast_to, node, "Type");
        dump_ast((const ast_node_t*) cast_node->opnd, node, "Opnd");
        break;
    case AST_EXPR_BINARY:
        const ast_expr_binary_t *binop_node = (const ast_expr_binary_t*) node;
        debug("%08x: EXPR_BINARY_%08x_%d\n", node, node, binop_node->op);
        dump_ast((const ast_node_t*) binop_node->left_opnd, node, "Lopnd");
        dump_ast((const ast_node_t*) binop_node->right_opnd, node, "Ropnd");
        break;
    case AST_EXPR_COND:
        break;
    case AST_EXPR_ASSIGN:
        break;
    case AST_EXPR_TYPESZ:
        break;
    case AST_EXPR_EXPRSZ:
        break;
    case AST_FUNC_IMPL:
        break;
    default:
        error("Unrecognized node type: %d\n", node->node_type);
        break;
    }
}
