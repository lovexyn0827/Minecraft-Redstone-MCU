#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include "tokenizer.h"

typedef struct symbol_t symbol_t;

typedef HASH_MAP_TYPE(str, symbol_t*) symbol_tbl_t;

typedef enum ast_node_type {
    AST_ROOT = 0x00000,
    AST_TYPENAME = 0x01000,
    AST_TYPE_SCALAR,
    AST_TYPE_FUNCT,
    AST_STMT = 0x02000,
    AST_STMT_DECL,
    AST_STMT_EXPR,
    AST_STMT_IF,
    AST_STMT_IFELSE,
    AST_STMT_FOR,
    AST_STMT_FORDECL,
    AST_STMT_WHILE,
    AST_STMT_DO,
    AST_STMT_SWITCH,
    AST_STMT_GOTO,
    AST_STMT_BREAK,
    AST_STMT_CONTINUE,
    AST_STMT_RETURN,
    AST_STMT_COMPOUND,
    AST_STMT_CASE,
    AST_STMT_DEFAULT,
    AST_STMT_LABELED,
    AST_DECL = 0x04000,
    AST_DECL_DRCT_FN,
    AST_DECL_DRCT_VAR,
    AST_EXPR = 0x08000,
    AST_EXPR_CALL,
    AST_EXPR_SYMBOL,
    AST_EXPR_CONST,
    AST_EXPR_UNARY,
    AST_EXPR_CAST,
    AST_EXPR_BINARY,
    AST_EXPR_COND,
    AST_EXPR_ASSIGN,
    AST_FUNC_IMPL = 0x10000,
} ast_node_type_t;

#define AST_NODE_SHARED_FIELDS \
    ast_node_type_t node_type;\
    const struct ast_node *parent;

typedef struct ast_node {
    AST_NODE_SHARED_FIELDS
} ast_node_t;

// *********** Type Names  ***********

#define AST_TYPENAME_NODE_SHARED_FIELDS \
    AST_NODE_SHARED_FIELDS

typedef struct ast_typename {
    AST_TYPENAME_NODE_SHARED_FIELDS
} ast_typename_t;

typedef enum elementary_type {
    ETYPE_INT_8 = TOKEN_KW_INT8_T,
    ETYPE_UINT_8 = TOKEN_KW_UINT8_T,
    ETYPE_POINTER
} elementary_type_t;

typedef struct ast_typename_scalar {
    AST_TYPENAME_NODE_SHARED_FIELDS
    elementary_type_t type_name;
    const struct ast_typename_scalar *underlying_type;
    bool immutable;
} ast_typename_scalar_t;

typedef struct ast_typename_funct {
    AST_TYPENAME_NODE_SHARED_FIELDS
    const ast_typename_t *return_type;
    ARRAY_LIST_TYPE(const ast_typename_t *) param_type;
} ast_typename_funct_t;

// *********** Expressions ***********

#define AST_EXPR_NODE_SHARED_FIELDS \
    AST_NODE_SHARED_FIELDS\
    bool constant;\
    bool lvalue;

typedef struct ast_expr {
    AST_EXPR_NODE_SHARED_FIELDS
} ast_expr_t;

typedef struct ast_expr_call {
    AST_EXPR_NODE_SHARED_FIELDS
    const ast_expr_t *function_addr;
    ARRAY_LIST_TYPE(const ast_expr_t*) arguments;
} ast_expr_call;

typedef struct ast_expr_symbol {
    AST_EXPR_NODE_SHARED_FIELDS
    const symbol_t *symbol;
} ast_expr_symbol_t;

typedef struct ast_expr_constant {
    AST_EXPR_NODE_SHARED_FIELDS
    uint32_t value;
} ast_expr_constant_t;

typedef enum unary_op {
    UOP_INCGET,
    UOP_DECGET,
    UOP_GETINC,
    UOP_GETDEC,
    UOP_ADDRESSOF,
    UOP_DEREFERENCE,
    UOP_POSITIVE,
    UOP_NEGATIVATE,
    UOP_BITWISE_NOT,
    UOP_LOGICAL_NOT
} unary_op_t;

typedef struct ast_expr_unary {
    AST_EXPR_NODE_SHARED_FIELDS
    unary_op_t op;
    const ast_expr_t *opnd;
} ast_expr_unary_t;

typedef struct ast_expr_cast {
    AST_EXPR_NODE_SHARED_FIELDS
    const ast_typename_t op;
    const ast_expr_t *opnd;
} ast_expr_cast_t;

typedef enum binary_op {
    BOP_MUL,
    BOP_DIV,
    BOP_MOD,
    BOP_ADD,
    BOP_SUB,
    BOP_SHL,
    BOP_SHR,
    BOP_LT,
    BOP_GT,
    BOP_LE,
    BOP_GE,
    BOP_EQ,
    BOP_NE,
    BOP_AND,
    BOP_XOR,
    BOP_OR,
    BOP_LAND,
    BOP_LOR
} binary_op_t;

typedef struct ast_expr_binary {
    AST_EXPR_NODE_SHARED_FIELDS
    binary_op_t op;
    const ast_expr_t *left_opnd;
    const ast_expr_t *right_opnd;
} ast_expr_binary_t;

typedef struct ast_expr_cond {
    AST_EXPR_NODE_SHARED_FIELDS
    const ast_expr_t *cond;
    const ast_expr_t *if_true;
    const ast_expr_t *if_false;
    bool likely_true;
} ast_expr_cond_t;

typedef enum assign_op {
    AOP_EQ,
    AOP_MULEQ,
    AOP_DIVEQ,
    AOP_MODEQ,
    AOP_ADDEQ,
    AOP_SUBEQ,
    AOP_SHLEQ,
    AOP_SHREQ,
    AOP_ANDEQ,
    AOP_OREQ,
    AOP_XOREQ,
    AOP_SET,
    AOP_CLR
} assign_op_t;

typedef struct ast_expr_assign {
    AST_EXPR_NODE_SHARED_FIELDS
    assign_op_t op;
    const ast_expr_t *left_opnd;
    const ast_expr_t *right_opnd;
} ast_expr_assign_t;

// *********** Declarations ***********

#define AST_DECL_SHARED_FIELDS \
    AST_NODE_SHARED_FIELDS

typedef struct ast_decl {
    AST_DECL_SHARED_FIELDS
} ast_decl_t;

typedef struct ast_decl_direct_function {
    AST_DECL_SHARED_FIELDS
    const ast_typename_funct_t *func_type;
    const symbol_t *func_name;
    const ast_expr_t *initializer;
    bool inline_func;
} ast_decl_direct_function_t;

typedef struct ast_decl_direct_variable {
    AST_DECL_SHARED_FIELDS
    const ast_typename_t *var_type;
    const symbol_t *var_name;
    const ast_expr_t *initializer;
    bool register_var;
} ast_decl_direct_variable_t;

// *********** Statements ***********

#define AST_STMT_NODE_SHARED_FIELDS \
    AST_NODE_SHARED_FIELDS

typedef struct ast_stmt {
    AST_NODE_SHARED_FIELDS
} ast_stmt_t;

typedef struct ast_stmt_decl {
    AST_NODE_SHARED_FIELDS
    const ast_decl_t *decl;
} ast_stmt_decl_t;

typedef struct ast_stmt_expr {
    AST_NODE_SHARED_FIELDS
    const ast_expr_t *expr;
} ast_stmt_expr_t;

typedef struct ast_stmt_compound {
    AST_NODE_SHARED_FIELDS
    ARRAY_LIST_TYPE(const ast_stmt_t *) statements;
    const uint_t entry_address;
    const uint_t end_address;
    symbol_tbl_t symbol_tbl;\
} ast_stmt_compound_t;

typedef struct ast_stmt_if {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *cond;
    const ast_stmt_compound_t *if_true;
    bool likely_true;
} ast_stmt_if_t;

typedef struct ast_stmt_ifelse {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *cond;
    const ast_stmt_compound_t *if_true;
    const ast_stmt_compound_t *if_false;
    bool likely_true;
} ast_stmt_ifelse_t;

typedef struct ast_stmt_for {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *init;
    const ast_expr_t *cond;
    const ast_expr_t *step;
    const ast_stmt_compound_t *body;
    bool likely_true;
} ast_stmt_for_t;

typedef struct ast_stmt_fordecl {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_decl_t *init;
    const ast_expr_t *cond;
    const ast_expr_t *step;
    const ast_stmt_compound_t *body;
    bool likely_true;
} ast_stmt_fordecl_t;

typedef struct ast_stmt_while {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *cond;
    const ast_stmt_compound_t *body;
    bool likely_true;
} ast_stmt_while_t;

typedef struct ast_stmt_do {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *cond;
    const ast_stmt_compound_t *body;
    bool likely_true;
} ast_stmt_do_t;

typedef struct ast_stmt_case {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *switch_on;
    const uint_t address;
    bool likely;
} ast_stmt_case_t;

typedef struct ast_stmt_default {
    AST_STMT_NODE_SHARED_FIELDS
    const uint_t address;
    bool likely;
} ast_stmt_default_t;

typedef struct ast_stmt_switch {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *switch_on;
    const ast_stmt_compound_t *body;
    HASH_MAP_TYPE(const ast_expr_t*, const ast_stmt_case_t*) cases;
    const ast_stmt_compound_t *default_case;
    bool likely_true;
} ast_stmt_switch_t;

typedef struct ast_stmt_goto {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *target;
    bool likely_true;
} ast_stmt_goto_t;

typedef struct ast_stmt_break {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_stmt_compound_t *from;
    bool likely_true;
} ast_stmt_break_t;

typedef struct ast_stmt_continue {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_stmt_compound_t *to;
    bool likely_true;
} ast_stmt_continue_t;

typedef struct ast_stmt_return {
    AST_STMT_NODE_SHARED_FIELDS
    const ast_expr_t *ret_value;
} ast_stmt_return_t;

typedef struct ast_stmt_labeled {
    AST_STMT_NODE_SHARED_FIELDS
    const symbol_t *label;
    const ast_stmt_t *underlying;
} ast_stmt_labeled_t;

// *********** Function Impl ***********

typedef struct ast_function_impl {
    AST_NODE_SHARED_FIELDS
    const ast_decl_direct_function_t *decl;
    const ast_stmt_compound_t *body;
    uint_t address;
} ast_function_impl_t;

// *********** AST ***********

typedef struct ast_root {
    AST_NODE_SHARED_FIELDS
    symbol_tbl_t symbol_tbl;
} ast_root_t;

typedef struct {
    ast_node_t root;
} ast_t;

#endif // AST_H_INCLUDED
