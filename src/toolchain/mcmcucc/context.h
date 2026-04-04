#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "common.h"
#include "array_list.h"
#include "map.h"
#include "ast.h"
#include "tokenizer.h"

typedef struct {
    const token_t **base;
    uint_t cur_pos;
    uint_t end_pos;
    ARRAY_LIST_TYPE(uint_t) marks;
} read_head_t;

typedef enum {
    SYM_FUNCTION = 1 << 0,
    SYM_VARIABLE = 1 << 1,
    SYM_LABEL = 1 << 2,
    SYM_NOTEXIST = 1 << 31
} symbol_type_t;

typedef enum obj_addr_space {
    OBJ_ADDR_IMM,
    OBJ_ADDR_REG,
    OBJ_ADDR_SRAM,
    OBJ_ADDR_IM,
    OBJ_ADDR_CSTACK,
    OBJ_ADDR_OSTACK,
    OBJ_ADDR_CSR,
    OBJ_ADDR_NIL
} obj_addr_space_t;

#define NIL_OBJ_ADDR (0xFFFFFFFF)

// Notes on semantics of the addr field
//  - For REG, denotes the index of a GPR.
//  - For SRAM & IM, denotes the address of a word in corresponding memory, or a unique label if addr[16] = 1.
//  - For CSR, denotes the address of a CSR.
//  - For IMM, denotes the unsigned value of a immediate.
//  - For CSTACK and OSTACK, the value of addr is silently ignored.
//  - If NIL_OBJ_ADDR were specified, the value of addr is considered invalid and needs further assignment(s).

typedef struct obj_addr {
    obj_addr_space_t type;
    uint_t addr;
} obj_addr_t;

typedef struct symbol_t {
    symbol_type_t type;
    str name;
    const ast_decl_t *decl;
    bool immutable;
    obj_addr_t *address;
} symbol_t;

typedef struct context {
    read_head_t *ptr;
    ast_node_t *cur_scope;
    ast_function_impl_t *cur_func;
    ast_t ast;
} context_t;

symbol_t *get_symbol(context_t *ctx, str name);
symbol_t *register_declared_symbol(context_t *ctx, str name, const ast_decl_t *decl);
symbol_t *register_label(context_t *ctx, str name);
void register_symbol(context_t *ctx, symbol_t *symb);

extern symbol_t NIL_SYMBOL;

void init_compilation_context(context_t *ctx, token_lst_t *tokens);

#endif // CONTEXT_H_INCLUDED
