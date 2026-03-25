#include "ast.h"

#include "common.h"
#include "context.h"

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
        ARRAY_LIST_TRAVERSE(fn_type_node->param_type, const ast_decl_direct_variable_t*, param, param_idx, {
            char_t arg_idx_buf[16];
            sprintf(arg_idx_buf, "Param_%d", param_idx);
            dump_ast((const ast_node_t*) param, node, arg_idx_buf);
        })
        break;
    case AST_STMT:
        break;
    case AST_STMT_DECL:
        const ast_stmt_decl_t *decl_node = (const ast_stmt_decl_t*) node;
        debug("%08x: DECL_%08x\n", node, node);
        dump_ast((const ast_node_t*) decl_node->decl, node, "Decl");
        break;
    case AST_STMT_EXPR:
        const ast_stmt_expr_t *expr_node = (const ast_stmt_expr_t*) node;
        debug("%08x: EXPR_%08x\n", node, node);
        dump_ast((const ast_node_t*) expr_node->expr, node, "Expr");
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
        const ast_decl_direct_function_t *fn_decl_node = (const ast_decl_direct_function_t*) node;
        debug("%08x: DECL_FN_%08x_%s_%s\n", node, node,
              fn_decl_node->decl_name->name, fn_decl_node->inline_func ? "L" : "");
        dump_ast((ast_node_t*) fn_decl_node->decl_type, node, "Type");
        if (fn_decl_node->initializer != NULL) {
            dump_ast((ast_node_t*) fn_decl_node->initializer, node, "Init");
        }

        break;
    case AST_DECL_DRCT_VAR:
        const ast_decl_direct_variable_t *var_decl_node = (const ast_decl_direct_variable_t*) node;
        debug("%08x: DECL_VAR_%08x_%s_%s\n", node, node,
              var_decl_node->decl_name->name, var_decl_node->register_var ? "R" : "");
        dump_ast((ast_node_t*) var_decl_node->decl_type, node, "Type");
        if (var_decl_node->initializer != NULL) {
            dump_ast((ast_node_t*) var_decl_node->initializer, node, "Init");
        }

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
        const ast_expr_symbol_t *sym_node = (const ast_expr_symbol_t*) node;
        debug("%08x: EXPR_SYMBOL_%08x_%s\n", node, node, sym_node->symbol->name);
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
        const ast_expr_assign_t *assign_node = (const ast_expr_assign_t*) node;
        debug("%08x: EXPR_ASSIGN_%08x_%d\n", node, node, assign_node->op);
        dump_ast((const ast_node_t*) assign_node->dest, node, "Dst");
        dump_ast((const ast_node_t*) assign_node->src, node, "Src");
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
