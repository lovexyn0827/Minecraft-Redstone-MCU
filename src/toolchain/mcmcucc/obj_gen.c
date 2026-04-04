#include "obj_gen.h"

// *********** Instruction Builder ***********

uint_t hash_insn_name(insn_name_t op) {
    return (uint_t) op;
}

bool insn_name_equal(insn_name_t i1, insn_name_t i2) {
    return i1 == i2;
}

HASH_MAP_TYPE(insn_name_t, str) INSN_NAMES;

void init_insn_name_map() {
    HASH_MAP_INIT(insn_name_t, str, INSN_NAMES, 64, hash_insn_name, NULL)
    HASH_MAP_PUT(INSN_NAMES, I_ADD, "ADD", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SUB, "SUB", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_AND, "AND", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_OR, "OR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_XOR, "XOR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SAR, "SAR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SHL, "SHL", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SHR, "SHR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SET, "SET", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_CLR, "CLR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_PUSH, "PUSH", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_POP, "POP", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_CMPU, "CMPU", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SARI, "SARI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SHLI, "SHLI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SHRI, "SHRI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_SETI, "SETI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_CLRI, "CLRI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_ILOAD, "ILOAD", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_ISTORE, "ISTORE", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_CMPIU, "CMPIU", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_ADDI, "ADDI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_ANDI, "ANDI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_ORI, "ORI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_XORI, "XORI", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_BEQZ, "BEQZ", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_BNEZ, "BNEZ", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_BGEZ, "BGEZ", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_BLTZ, "BLTZ", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_RET, "RET", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_JMP, "JMP", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_INCSR, "INCSR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_OUTCSR, "OUTCSR", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_LJMP, "LJMP", insn_name_t, str, insn_name_equal)
    HASH_MAP_PUT(INSN_NAMES, I_INVOKE, "INVOKE", insn_name_t, str, insn_name_equal)
}

str get_insn_name(insn_name_t op) {
    str name;
    HASH_MAP_GET(INSN_NAMES, op, name, insn_name_t, str, insn_name_equal)
    return name;
}

void build_alu_insn(insn_name_t op, reg_no_t rs, reg_no_t rt, reg_no_t rd, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\tr%d\tr%d\n", get_insn_name(op), rd, rs, rt);
}

void build_immalu_insn(insn_name_t op, reg_no_t rs, reg_no_t rt, uint_t imm, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\tr%d\n", get_insn_name(op), rt, rs);
}

void build_memory_insn(insn_name_t op, reg_no_t rs, reg_no_t rt, uint_t offset, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\tr%d(%#02x)\n", get_insn_name(op), rt, rs, offset);
}

void build_stack_insn(insn_name_t op, reg_no_t reg, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\n", get_insn_name(op), reg);
}

void build_jmp_insn(insn_name_t op, uint_t offset, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8s%#02x\n", get_insn_name(op), offset);
}

void build_jmpreg_insn(insn_name_t op, reg_no_t rs, uint_t offset, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d(%#02x)\n", get_insn_name(op), rs, offset);
}

void build_branch_insn(insn_name_t op, reg_no_t rs, uint_t offset, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\t%#02x\n", get_insn_name(op), rs, offset);
}

void build_ret_insn(insn_name_t op, bool reti, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8s%c\n", get_insn_name(op), reti ? 'I' : ' ');
}

str get_cmp_mode_name(cmp_mode_t cmp_mode) {
    switch (cmp_mode) {
    case CMP_EQ:
        return "EQ";
    case CMP_NE:
        return "NE";
    case CMP_GT:
        return "GT";
    case CMP_LT:
        return "LT";
    default:
        return NULL;    // Expecting the program to fail
    }
}

void build_cmp_insn(insn_name_t op, reg_no_t rs, reg_no_t rt, reg_no_t rd, cmp_mode_t cmp_mode, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\tr%d\t%s\tr%d\n", get_insn_name(op), rd, rs, get_cmp_mode_name(cmp_mode), rt);
}

void build_cmpi_insn(insn_name_t op, reg_no_t rs, reg_no_t rt, uint_t imm, cmp_mode_t cmp_mode, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\tr%d\t%s\t%#02x\n", get_insn_name(op), rt, rs, get_cmp_mode_name(cmp_mode), imm);
}

void build_io_insn(insn_name_t op, reg_no_t reg, uint_t io_port, FILE *asm_dest) {
    fprintf(asm_dest, "\t%8sr%d\t%#03x\n", get_insn_name(op), reg, io_port);
}

// *********** Expression Builder ***********

void build_unary_expr_evaluator(context_t *ctx, const ast_expr_unary_t *expr,
                                const obj_addr_t *val_dest, FILE *asm_dest) {
    // ++   --  &   *   +   -   ~   !
    switch (expr->op) {
    case UOP_INCGET:
        break;
    case UOP_GETINC:
        break;
    case UOP_DECGET:
        break;
    case UOP_GETDEC:
        break;
    case UOP_ADDRESSOF:
        break;
    case UOP_DEREFERENCE:
        break;
    case UOP_POSITIVE:
        break;
    case UOP_NEGATIVATE:
        break;
    case UOP_BITWISE_NOT:
        break;
    case UOP_LOGICAL_NOT:
    break;
    default:
        fatal("No way! Unexpected op!");
    }
}

void build_binary_expr_evaluator(context_t *ctx, const ast_expr_binary_t *expr,
                                 const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_cond_expr_evaluator(context_t *ctx, const ast_expr_cond_t *expr,
                               const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_assign_expr_evaluator(context_t *ctx, const ast_expr_assign_t *expr,
                                 const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_call_expr_evaluator(context_t *ctx, const ast_expr_call_t *expr,
                               const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_symbol_expr_evaluator(context_t *ctx, const ast_expr_symbol_t *expr,
                                 const obj_addr_t *val_dest, FILE *asm_dest) {
}

void build_expr_evaluator(context_t *ctx, const ast_expr_t *expr, const obj_addr_t *val_dest, FILE *asm_dest) {
    switch (expr->node_type) {
    case AST_EXPR_UNARY:
        build_unary_expr_evaluator(ctx, (const ast_expr_unary_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_BINARY:
        build_binary_expr_evaluator(ctx, (const ast_expr_binary_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_COND:
        build_cond_expr_evaluator(ctx, (const ast_expr_cond_t*) expr, val_dest, asm_dest);
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
        build_assign_expr_evaluator(ctx, (const ast_expr_assign_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_CALL:
        build_call_expr_evaluator(ctx, (const ast_expr_call_t*) expr, val_dest, asm_dest);
        break;
    case AST_EXPR_CONST:
        // Automatically handled in involved insns
        break;
    case AST_EXPR_SYMBOL:
        build_symbol_expr_evaluator(ctx, (const ast_expr_symbol_t*) expr, val_dest, asm_dest);
        break;
    default:
        fatal("Expected expr nodes!\n");
    }
}

void generate_asm(context_t *ctx, FILE *asm_dest) {
}
