#ifndef OBJ_GEN_H_INCLUDED
#define OBJ_GEN_H_INCLUDED

#include "context.h"

#include <stdio.h>

// We're not generating binaries directly thus we don't have to include binary opcode here.

typedef enum insn_name {
    I_ADD,
    I_SUB,
    I_AND,
    I_OR,
    I_XOR,
    I_SAR,
    I_SHL,
    I_SHR,
    I_SET,
    I_CLR,
    I_PUSH,
    I_POP,
    I_CMPU,
    I_SARI,
    I_SHLI,
    I_SHRI,
    I_SETI,
    I_CLRI,
    I_ILOAD,
    I_ISTORE,
    I_CMPIU,
    I_ADDI,
    I_ANDI,
    I_ORI,
    I_XORI,
    I_BEQZ,
    I_BNEZ,
    I_BGEZ,
    I_BLTZ,
    I_RET,
    I_JMP,
    I_INCSR,
    I_OUTCSR,
    I_LJMP,
    I_INVOKE
} insn_name_t;

typedef uint_t reg_no_t;

typedef enum cmp_mode {
    CMP_EQ,
    CMP_NE,
    CMP_GT,
    CMP_LT
} cmp_mode_t;

void generate_asm(parse_ctx_t *ctx, FILE *dest);

#endif // OBJ_GEN_H_INCLUDED
