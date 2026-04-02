#include "ast.h"

#include "common.h"
#include "context.h"

#include <stdio.h>

void dump_ast(const ast_node_t *node, const ast_node_t *parent, str field) {
    debug("%08x --> %08x: %s\n", parent, node, field);
    switch (node->node_type) {
    case AST_ROOT:
        debug("%08x: ROOT_%08x\n", node, node);
        const ast_root_t *root = (const ast_root_t*) node;
        ARRAY_LIST_TRAVERSE(root->children, const ast_node_t*, child, child_idx, {
            char_t child_idx_buf[16];
            sprintf(child_idx_buf, "Child_%d", child_idx);
            dump_ast(child, node, child_idx_buf);
        })
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
        ARRAY_LIST_TRAVERSE(decl_node->declarators, const ast_decl_t*, decl, decl_idx, {
            char_t decl_idx_buf[16];
            sprintf(decl_idx_buf, "Decl_%d", decl_idx);
            dump_ast((const ast_node_t*) decl, node, decl_idx_buf);
        })
        break;
    case AST_STMT_EXPR:
        const ast_stmt_expr_t *expr_node = (const ast_stmt_expr_t*) node;
        debug("%08x: EXPR_%08x\n", node, node);
        dump_ast((const ast_node_t*) expr_node->expr, node, "Expr");
        break;
    case AST_STMT_EMPTY:
        debug("%08x: STMT_EMPTY_%08x\n", node, node);
        break;
    case AST_STMT_IF:
        const ast_stmt_if_t *if_stmt = (const ast_stmt_if_t*) node;
        debug("%08x: IF_%08x_%s\n", node, node, if_stmt->likely_true ? "L" : "U");
        dump_ast((ast_node_t*) if_stmt->cond, node, "Cond");
        dump_ast((ast_node_t*) if_stmt->if_true, node, "IfTrue");
        if (if_stmt->if_false != NULL) {
            dump_ast((ast_node_t*) if_stmt->if_false, node, "IfFalse");
        }

        break;
    case AST_STMT_FOR:
        const ast_stmt_for_t *for_stmt = (const ast_stmt_for_t*) node;
        debug("%08x: FOR_%08x_%s\n", node, node, for_stmt->likely_true ? "L" : "U");
        dump_ast((ast_node_t*) for_stmt->init, node, "Init");
        dump_ast((ast_node_t*) for_stmt->cond, node, "Cond");
        dump_ast((ast_node_t*) for_stmt->step, node, "Step");
        dump_ast((ast_node_t*) for_stmt->body, node, "Body");
        break;
    case AST_STMT_FORDECL:
        const ast_stmt_fordecl_t *fordecl_stmt = (const ast_stmt_fordecl_t*) node;
        debug("%08x: FOR_%08x_%s\n", node, node, fordecl_stmt->likely_true ? "L" : "U");
        dump_ast((ast_node_t*) fordecl_stmt->init, node, "Init");
        dump_ast((ast_node_t*) fordecl_stmt->cond, node, "Cond");
        dump_ast((ast_node_t*) fordecl_stmt->step, node, "Step");
        dump_ast((ast_node_t*) fordecl_stmt->body, node, "Body");
        break;
    case AST_STMT_WHILE:
        const ast_stmt_while_t *while_stmt = (const ast_stmt_while_t*) node;
        debug("%08x: WHILE_%08x_%s\n", node, node, while_stmt->likely_true ? "L" : "U");
        dump_ast((ast_node_t*) while_stmt->cond, node, "Cond");
        dump_ast((ast_node_t*) while_stmt->body, node, "Body");
        break;
    case AST_STMT_DO:
        const ast_stmt_do_t *do_stmt = (const ast_stmt_do_t*) node;
        debug("%08x: DO_%08x_%s\n", node, node, do_stmt->likely_true ? "L" : "U");
        dump_ast((ast_node_t*) do_stmt->cond, node, "Cond");
        dump_ast((ast_node_t*) do_stmt->body, node, "Body");
        break;
    case AST_STMT_SWITCH:
        const ast_stmt_switch_t *switch_stmt = (const ast_stmt_switch_t*) node;
        debug("%08x: SWITCH_%08x\n", node, node);
        dump_ast((ast_node_t*) switch_stmt->switch_on, node, "On");
        dump_ast((ast_node_t*) switch_stmt->body, node, "Body");
        break;
    case AST_STMT_GOTO:
        const ast_stmt_goto_t *goto_stmt = (const ast_stmt_goto_t*) node;
        debug("%08x: GOTO_%08x\n", node, node);
        dump_ast((ast_node_t*) goto_stmt->target, node, "Target");
        break;
    case AST_STMT_BREAK:
        debug("%08x: BREAK_%08x\n", node, node);
        break;
    case AST_STMT_CONTINUE:
        debug("%08x: CONTINUE_%08x\n", node, node);
        break;
    case AST_STMT_RETURN:
        const ast_stmt_return_t* ret_stmt = (const ast_stmt_return_t*) node;
        debug("%08x: RETURN_%08x\n", node, node);
        dump_ast((ast_node_t*) ret_stmt->ret_value, node, "Rets");
        break;
    case AST_STMT_COMPOUND:
        const ast_stmt_compound_t *comp_stmt = (const ast_stmt_compound_t*) node;
        debug("%08x: STMT_COMP_%08x\n", node, node);
        ARRAY_LIST_TRAVERSE(comp_stmt->statements, const ast_stmt_t*, stmt, stmt_idx, {
            char_t arg_idx_buf[16];
            sprintf(arg_idx_buf, "Stmt_%d", stmt_idx);
            dump_ast((const ast_node_t*) stmt, node, arg_idx_buf);
        })
        break;
    case AST_STMT_CASE:
        const ast_stmt_case_t *case_stmt = (const ast_stmt_case_t*) node;
        debug("%08x: CASE_%08x_%s\n", node, node, case_stmt->likely ? "L" : "U");
        dump_ast((const ast_node_t*) case_stmt->switch_on, node, "Val");
        break;
    case AST_STMT_DEFAULT:
        const ast_stmt_default_t *default_stmt = (const ast_stmt_default_t*) node;
        debug("%08x: DEFAULT_%08x_%s\n", node, node, default_stmt->likely ? "L" : "U");
        break;
    case AST_STMT_LABELED:
        const ast_stmt_labeled_t *lbld_stmt = (const ast_stmt_labeled_t*) node;
        debug("%08x: LABEL_%08x_%s\n", node, node, lbld_stmt->label->name);
        dump_ast((const ast_node_t*) lbld_stmt->underlying, node, "Stmt");
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
        const ast_expr_cond_t *cond_expr = (const ast_expr_cond_t*) node;
        debug("%08x: COND_%08x_%s\n", node, node, cond_expr->likely_true ? "L" : "U");
        dump_ast((const ast_node_t*) cond_expr->cond, node, "Cond");
        dump_ast((const ast_node_t*) cond_expr->if_true, node, "IfTrue");
        dump_ast((const ast_node_t*) cond_expr->if_false, node, "IfFalse");
        break;
    case AST_EXPR_ASSIGN:
        const ast_expr_assign_t *assign_node = (const ast_expr_assign_t*) node;
        debug("%08x: EXPR_ASSIGN_%08x_%d\n", node, node, assign_node->op);
        dump_ast((const ast_node_t*) assign_node->dest, node, "Dst");
        dump_ast((const ast_node_t*) assign_node->src, node, "Src");
        break;
    case AST_EXPR_TYPESZ:
        const ast_expr_sizeof_expr_t* exprsz_expr = (const ast_expr_sizeof_expr_t*) node;
        debug("%08d: SIZEOF_%08d\n");
        dump_ast((const ast_node_t*) exprsz_expr->sizeof_expr, node, "Of");
        break;
    case AST_EXPR_EXPRSZ:
        const ast_expr_sizeof_type_t* typesz_expr = (const ast_expr_sizeof_type_t*) node;
        debug("%08d: SIZEOF_%08d\n");
        dump_ast((const ast_node_t*) typesz_expr->sizeof_type, node, "Of");
        break;
    case AST_FUNC_IMPL:
        const ast_function_impl_t *fn_impl = (const ast_function_impl_t*) node;
        debug("%08x: EXPR_SYMBOL_%08x_%s\n", node, node, fn_impl->decl->decl_name->name);
        dump_ast((const ast_node_t*) fn_impl->decl, node, "Decl");
        dump_ast((const ast_node_t*) fn_impl->body, node, "Body");
        break;
    default:
        error("Unrecognized node type: %d\n", node->node_type);
        break;
    }
}

#define REPLACE_NODE_FIELD(F) {\
    ast_node_t *new_node = for_each_node((ast_node_t*) (F), before_children, after_children);\
    if (new_node != NULL) {\
        (F) = (typeof(F)) new_node;\
    }\
}

#define REPLACE_NODE_FIELD_IN_LIST(L, F, I) {\
    ast_node_t *new_node = for_each_node((ast_node_t*) (F), before_children, after_children);\
    if (new_node != NULL) {\
        ARRAY_LIST_SET(L, F, I)\
    }\
}

ast_node_t *for_each_node(ast_node_t *node, node_visitor before_children, node_visitor after_children) {
    if (node == NULL) {
        return NULL;
    }

    if (before_children != NULL) {
        ast_node_t *new_node = before_children(node);
        if (new_node != NULL) {
            node = new_node;
        }
    }

    switch (node->node_type) {
    case AST_ROOT:
        ast_root_t *root = (ast_root_t*) node;
        ARRAY_LIST_TRAVERSE(root->children, ast_node_t*, child, child_idx, {
            REPLACE_NODE_FIELD_IN_LIST(root->children, child, child_idx)
        })
        break;
    case AST_TYPENAME:
        break;
    case AST_TYPE_PRIM:
        break;
    case AST_TYPE_PTR:
        ast_typename_ptr_t *ptr_type_node = (ast_typename_ptr_t*) node;
        REPLACE_NODE_FIELD(ptr_type_node->underlying_type)
        break;
    case AST_TYPE_FUNCT:
        ast_typename_funct_t *fn_type_node = (ast_typename_funct_t*) node;
        REPLACE_NODE_FIELD(fn_type_node->return_type)
        ARRAY_LIST_TRAVERSE(fn_type_node->param_type, ast_decl_direct_variable_t*, param, param_idx, {
            REPLACE_NODE_FIELD_IN_LIST(fn_type_node->param_type, param, param_idx)
        })
        break;
    case AST_STMT:
        break;
    case AST_STMT_DECL:
        ast_stmt_decl_t *decl_node = (ast_stmt_decl_t*) node;
        ARRAY_LIST_TRAVERSE(decl_node->declarators, ast_decl_t*, decl, decl_idx, {
            REPLACE_NODE_FIELD_IN_LIST(decl_node->declarators, decl, decl_idx)
        })
        break;
    case AST_STMT_EXPR:
        ast_stmt_expr_t *expr_node = (ast_stmt_expr_t*) node;
        REPLACE_NODE_FIELD(expr_node->expr)
        break;
    case AST_STMT_EMPTY:
        break;
    case AST_STMT_IF:
        ast_stmt_if_t *if_stmt = (ast_stmt_if_t*) node;
        REPLACE_NODE_FIELD(if_stmt->cond)
        REPLACE_NODE_FIELD(if_stmt->if_true)
        REPLACE_NODE_FIELD(if_stmt->if_false)
        break;
    case AST_STMT_FOR:
        ast_stmt_for_t *for_stmt = (ast_stmt_for_t*) node;
        REPLACE_NODE_FIELD(for_stmt->init)
        REPLACE_NODE_FIELD(for_stmt->cond)
        REPLACE_NODE_FIELD(for_stmt->step)
        REPLACE_NODE_FIELD(for_stmt->body)
        break;
    case AST_STMT_FORDECL:
        ast_stmt_fordecl_t *fordecl_stmt = (ast_stmt_fordecl_t*) node;
        REPLACE_NODE_FIELD(fordecl_stmt->init)
        REPLACE_NODE_FIELD(fordecl_stmt->cond)
        REPLACE_NODE_FIELD(fordecl_stmt->step)
        REPLACE_NODE_FIELD(fordecl_stmt->body)
        break;
    case AST_STMT_WHILE:
        ast_stmt_while_t *while_stmt = (ast_stmt_while_t*) node;
        REPLACE_NODE_FIELD(while_stmt->cond)
        REPLACE_NODE_FIELD(while_stmt->body)
        break;
    case AST_STMT_DO:
        ast_stmt_do_t *do_stmt = (ast_stmt_do_t*) node;
        REPLACE_NODE_FIELD(do_stmt->cond)
        REPLACE_NODE_FIELD(do_stmt->body)
        break;
    case AST_STMT_SWITCH:
        ast_stmt_switch_t *switch_stmt = (ast_stmt_switch_t*) node;
        REPLACE_NODE_FIELD(switch_stmt->switch_on)
        REPLACE_NODE_FIELD(switch_stmt->body)
        break;
    case AST_STMT_GOTO:
        ast_stmt_goto_t *goto_stmt = (ast_stmt_goto_t*) node;
        REPLACE_NODE_FIELD(goto_stmt->target)
        break;
    case AST_STMT_BREAK:
        break;
    case AST_STMT_CONTINUE:
        break;
    case AST_STMT_RETURN:
        ast_stmt_return_t* ret_stmt = (ast_stmt_return_t*) node;
        REPLACE_NODE_FIELD(ret_stmt->ret_value)
        break;
    case AST_STMT_COMPOUND:
        ast_stmt_compound_t *comp_stmt = (ast_stmt_compound_t*) node;
        ARRAY_LIST_TRAVERSE(comp_stmt->statements, ast_stmt_t*, stmt, stmt_idx, {
            REPLACE_NODE_FIELD_IN_LIST(comp_stmt->statements, stmt, stmt_idx)
        })
        break;
    case AST_STMT_CASE:
        ast_stmt_case_t *case_stmt = (ast_stmt_case_t*) node;
        REPLACE_NODE_FIELD(case_stmt->switch_on)
        break;
    case AST_STMT_DEFAULT:
        break;
    case AST_STMT_LABELED:
        ast_stmt_labeled_t *lbld_stmt = (ast_stmt_labeled_t*) node;
        REPLACE_NODE_FIELD(lbld_stmt->underlying)
        break;
    case AST_DECL:
        break;
    case AST_DECL_DRCT_FN:
        ast_decl_direct_function_t *fn_decl_node = (ast_decl_direct_function_t*) node;
        REPLACE_NODE_FIELD(fn_decl_node->decl_type)
        REPLACE_NODE_FIELD(fn_decl_node->initializer)
        break;
    case AST_DECL_DRCT_VAR:
        ast_decl_direct_variable_t *var_decl_node = (ast_decl_direct_variable_t*) node;
        REPLACE_NODE_FIELD(var_decl_node->decl_type)
        REPLACE_NODE_FIELD(var_decl_node->initializer)
        break;
    case AST_EXPR:
        break;
    case AST_EXPR_CALL:
        ast_expr_call_t *call_node = (ast_expr_call_t*) node;
        REPLACE_NODE_FIELD(call_node->function_addr)
        ARRAY_LIST_TRAVERSE(call_node->arguments, ast_expr_t*, arg, arg_idx, {
            REPLACE_NODE_FIELD_IN_LIST(call_node->arguments, arg, arg_idx)
        })
        break;
    case AST_EXPR_SYMBOL:
        break;
    case AST_EXPR_CONST:
        break;
    case AST_EXPR_UNARY:
        ast_expr_unary_t *unary_node = (ast_expr_unary_t*) node;
        for_each_node((ast_node_t*)unary_node->opnd, before_children, after_children);
        break;
    case AST_EXPR_CAST:
        ast_expr_cast_t *cast_node = (ast_expr_cast_t*) node;
        REPLACE_NODE_FIELD(cast_node->cast_to)
        REPLACE_NODE_FIELD(cast_node->opnd)
        break;
    case AST_EXPR_BINARY:
        ast_expr_binary_t *binop_node = (ast_expr_binary_t*) node;
        REPLACE_NODE_FIELD(binop_node->left_opnd)
        REPLACE_NODE_FIELD(binop_node->right_opnd)
        break;
    case AST_EXPR_COND:
        ast_expr_cond_t *cond_expr = (ast_expr_cond_t*) node;
        REPLACE_NODE_FIELD(cond_expr->cond)
        REPLACE_NODE_FIELD(cond_expr->if_true)
        REPLACE_NODE_FIELD(cond_expr->if_false)
        break;
    case AST_EXPR_ASSIGN:
        ast_expr_assign_t *assign_node = (ast_expr_assign_t*) node;
        REPLACE_NODE_FIELD(assign_node->dest)
        REPLACE_NODE_FIELD(assign_node->src)
        break;
    case AST_EXPR_TYPESZ:
        ast_expr_sizeof_expr_t* exprsz_expr = (ast_expr_sizeof_expr_t*) node;
        REPLACE_NODE_FIELD(exprsz_expr->sizeof_expr)
        break;
    case AST_EXPR_EXPRSZ:
        ast_expr_sizeof_type_t* typesz_expr = (ast_expr_sizeof_type_t*) node;
        REPLACE_NODE_FIELD(typesz_expr->sizeof_type)
        break;
    case AST_FUNC_IMPL:
        ast_function_impl_t *fn_impl = (ast_function_impl_t*) node;
        REPLACE_NODE_FIELD(fn_impl->decl)
        REPLACE_NODE_FIELD(fn_impl->body)
        break;
    default:
        error("Unrecognized node type: %d\n", node->node_type);
        break;
    }

    if (after_children != NULL) {
        ast_node_t *new_node = after_children(node);
        if (new_node != NULL) {
            node = new_node;
        }
    }

    return node;
}
