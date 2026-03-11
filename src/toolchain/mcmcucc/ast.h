#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

typedef struct symbol_t symbol_t;

typedef enum {
    AST_ROOT,
    AST_BLOCK_IF,
    AST_BLOCK_FOR,
    AST_BLOCK_WHILE,
    AST_BLOCK_DO,
    AST_BLOCK_STUB,
    AST_STATEMENT,
    AST_FN_DECL,
    AST_FN_IMPL,
    AST_FN_CALL,
    AST_VAR_DECL,
    AST_VAR_REF,
    AST_NUM_LITERAL,
    AST_BINARYOP,
    AST_EXPR_RVALUE,
    AST_EXPR_LVALUE
} ast_node_type_t;

#define AST_NODE_SHARED_FIELDS \
    ast_node_type_t type;\
    struct ast_node_t *parent;\
    HASH_MAP_TYPE(str, symbol_t*) symbol_tbl;\
    ARRAY_LIST_TYPE(struct ast_node_t *) children;

typedef struct ast_node_t {
    AST_NODE_SHARED_FIELDS
} ast_node_t;

typedef struct ast_var_ref {
    AST_NODE_SHARED_FIELDS
    symbol_t *variable;
} ast_var_ref_t;

typedef struct ast_num_literal {
    AST_NODE_SHARED_FIELDS
    uint_t value;
} ast_num_literal_t;

typedef struct ast_expr {
    AST_NODE_SHARED_FIELDS
} ast_expr_t;

typedef struct ast_block_if {
    AST_NODE_SHARED_FIELDS
    ast_expr_t *cond;
} ast_block_if_t;

typedef struct {
    ast_node_t root;
} ast_t;

#endif // AST_H_INCLUDED
