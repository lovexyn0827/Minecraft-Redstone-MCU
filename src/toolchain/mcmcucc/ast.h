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
    AST_FN_DECL,
    AST_FN_IMPL,
    AST_VAR_DECL,
    AST_STATEMENT,
    AST_AL_EXPR,
    AST_CALL,
    AST_SUBSCRIPT
} ast_node_type_t;

typedef struct ast_node_t {
    ast_node_type_t type;
    struct ast_node_t *parent;
    HASH_MAP_TYPE(str, symbol_t*) symbol_tbl;
    union {
    } content;
    ARRAY_LIST_TYPE(struct ast_node_t *) children;
} ast_node_t;

typedef struct {
    ast_node_t root;
} ast_t;

#endif // AST_H_INCLUDED
