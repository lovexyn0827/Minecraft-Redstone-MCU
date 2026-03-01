#include <stdlib.h>

#include "parser.h"
#include "string.h"

#include "const_expression.h"
#include "instructions.h"

/*********** Token Stream Reader ***********/

bool inbounds(read_head *ptr) {
    return ptr->ptr < ptr->max;
}

bool inbounds_after_n(read_head *ptr, uint_t offset) {
    return (ptr->ptr + offset) < ptr->max;
}

void assert_inbounds(read_head *ptr) {
    if (!inbounds(ptr)) {
        error("Unexpected end of input.\n");
    }
}

str peek_current(read_head *ptr) {
    return inbounds(ptr) ? *(ptr->ptr) : NULL;
}

str peek_current_plus_n(read_head *ptr, uint_t offset) {
    return inbounds_after_n(ptr, offset) ? *(ptr->ptr + offset) : NULL;
}

str read_current(read_head *ptr) {
    // printf(peek_current(ptr));
    return inbounds(ptr) ? *((ptr->ptr)++) : NULL;
}

bool cur_token_match(read_head *ptr, str against) {
    str cur_token = peek_current(ptr);
    return cur_token != NULL && str_equal(cur_token, against);
}

bool cur_token_match_n(read_head *ptr, str against, uint_t n) {
    str cur_token = peek_current_plus_n(ptr, n);
    return cur_token != NULL && str_equal(cur_token, against);
}

bool at_line_end(read_head *ptr) {
    return !inbounds(ptr) || *peek_current(ptr) == '\n';
}

void step_to_next_line(read_head *ptr) {
    while (!at_line_end(ptr)) {
        skip_current(ptr);
    }

    skip_current(ptr);
}

void skip_current(read_head *ptr) {
    // printf(peek_current(ptr));
    if (inbounds(ptr)) (ptr->ptr)++;
}

/*********** Compilation Context Management ***********/

void init_parse_ctx(parse_ctx *ctx, uint_t *insn_buf) {
    ctx->next_var_ptr = 0x00;
    ctx->next_insn_ptr = PROG_ENTRY;
    ctx->insns = insn_buf;
    ctx->sram_memmap = (bool*) malloc(SRAM_CAPACITY * sizeof(bool));
    ctx->im_memmap = (bool*) malloc(IM_CAPACITY * sizeof(bool));
    for (int i = 0; i < SRAM_CAPACITY; i++) ctx->sram_memmap[i] = false;
    for (int i = 0; i < IM_CAPACITY; i++) ctx->im_memmap[i] = false;
    HASH_MAP_INIT(str, int_t, ctx->constants, 64, hash_str, UNDEFINED_CONSTANT);
    HASH_MAP_INIT(str, uint_t, ctx->pointers, 64, hash_str, UNDEFINED_CONSTANT);
    HASH_MAP_INIT(str, uint_t, ctx->labels, 64, hash_str, UNDEFINED_CONSTANT);
}

void free_parse_ctx(parse_ctx *ctx) {
    free(ctx->sram_memmap);
    free(ctx->im_memmap);
    HASH_MAP_FREE(ctx->constants);
    HASH_MAP_FREE(ctx->pointers);
    HASH_MAP_FREE(ctx->labels);
}

void save_constant(parse_ctx *ctx, str name, int_t value) {
    bool succ;
    HASH_MAP_PUT_RET(ctx->constants, name, value, str, int_t, str_equal, succ);
    if (!succ) error("Constant %s has already been defined!", name);
}

void save_pointer(parse_ctx *ctx, str name, uint_t value) {
    bool succ;
    HASH_MAP_PUT_RET(ctx->pointers, name, value, str, int_t, str_equal, succ);
    if (!succ) error("Pointer / Variable %s has already been defined!", name);
}

void save_label(parse_ctx *ctx, str name, uint_t pos) {
    bool succ;
    HASH_MAP_PUT_RET(ctx->labels, name, pos, str, int_t, str_equal, succ);
    if (!succ) error("Label %s has already been defined!", name);
}

int_t get_constant(parse_ctx *ctx, str name) {
    uint_t const_val;
    HASH_MAP_GET(ctx->constants, name, const_val, str, uint_t, str_equal);
    return const_val;
}

uint_t get_pointer(parse_ctx *ctx, str name) {
    uint_t const_val;
    HASH_MAP_GET(ctx->pointers, name, const_val, str, uint_t, str_equal);
    return const_val;
}

uint_t get_label(parse_ctx *ctx, str name) {
    uint_t const_val;
    HASH_MAP_GET(ctx->labels, name, const_val, str, uint_t, str_equal);
    return const_val;
}

uint_t allocate_variable(parse_ctx *ctx, uint_t size) {
    // TODO Clamp space
    uint_t pos = ctx->next_var_ptr;
    for (uint_t i = 0; i < size; i++) {
        uint_t addr = pos + i;
        if (ctx->sram_memmap[addr]) {
            error("SRAM overlapped @ 0x%02x", addr);
        } else {
            ctx->sram_memmap[addr] = true;
        }
    }

    ctx->next_var_ptr += size;
    if (ctx->next_var_ptr > SRAM_CAPACITY) error("Variable @ 0x%02x + 0x%02x is out of bound!", pos, size);
    return pos;
}

/*********** Instruction Field Parsers ***********/

void parse_label(parse_ctx *ctx, read_head *ptr) {
    str label_name = read_current(ptr);
    save_label(ctx, label_name, ctx->next_insn_ptr);
    skip_current(ptr);
}

bool at_reg_no(read_head *ptr) {
    if (!inbounds(ptr)) {
        return false;
    }

    str regno_str = peek_current(ptr);
    if (strlen(regno_str) < 2 || strlen(regno_str) > 3) {
        return false;
    }

    if (tolower(regno_str[0]) != 'r') {
        return false;
    }

    int regno = strtol(regno_str + 1, NULL, 10);
    if (regno > MAX_REGNUM) {
        return false;
    }

    return true;
}

uint_t parse_reg_no(read_head *ptr) {
    assert_inbounds(ptr);
    str regno_str = read_current(ptr);
    if (strlen(regno_str) < 2 || strlen(regno_str) > 3) {
        error("Invalid register number: %s\n", regno_str);
    }

    if (tolower(regno_str[0]) != 'r') {
        error("Invalid register number: %s\n", regno_str);
    }

    int regno = strtol(regno_str + 1, NULL, 10);
    if (regno > MAX_REGNUM) {
        error("Invalid register number: %s\n", regno_str);
    }

    return regno;
}

bool is_bit_manip_insn(uint_t opcode) {
    return opcode == OP_BITMANIP;
}

// MemAddr := Variable | Reg\(Expr\)

void parse_reg_addr_pair(parse_ctx *ctx, read_head *ptr, uint_t *reg, uint_t *imm, uint_t imm_mask) {
    *reg = parse_reg_no(ptr);
    skip_current(ptr);  // skips '('
    *imm = eval_const_expr(ctx, ptr) & imm_mask;
    skip_current(ptr);  // skips ')'
}

void parse_mem_addr(parse_ctx *ctx, read_head *ptr, uint_t *rs, uint_t *imm) {
    if (at_reg_no(ptr)) {
        parse_reg_addr_pair(ctx, ptr, rs, imm, 0xFF);
    } else {
        *rs = 0;
        *imm = eval_const_expr(ctx, ptr);
    }
}

// JmpRegTarget := Reg\(Expr\)

void parse_jmp_reg_target(parse_ctx *ctx, read_head *ptr, uint_t *rs, uint_t *imm) {
    // Indirect jumps are absolute
    if (at_reg_no(ptr)) {
        parse_reg_addr_pair(ctx, ptr, rs, imm, 0x3FF);
    } else {
        *rs = 0;
        *imm = eval_const_expr(ctx, ptr) & 0x3FF;
    }
}

// JmpTarget := Expr

uint_t parse_jmp_target(parse_ctx *ctx, read_head *ptr) {
    // Direct jumps are relative
    uint_t base = ctx->next_insn_ptr;
    return (eval_const_expr(ctx, ptr) - base) & 0x3FF;
}

// Cond := EQ | NE | GT | LT

HASH_MAP_TYPE(str, uint_t) COND_CODES;

uint_t parse_cond_code(read_head *ptr) {
    uint_t cond_code;
    HASH_MAP_GET(COND_CODES, read_current(ptr), cond_code, str, uint_t, str_equal)
    if (cond_code == COND_INVALID) error("Unrecognized condition code: %s\n", peek_current_plus_n(ptr, -1));
    return cond_code;
}

// IOPort := Expr

uint_t parse_io_port(parse_ctx *ctx, read_head *ptr) {
    return eval_const_expr(ctx, ptr) & 0x3FF;
}

/*********** Instruction Parsers ***********/

insn_t gen_r_type_insn(uint_t opcode, uint_t rs, uint_t rt, uint_t rd, uint_t funct) {
    return (opcode << 16) | (rs << 12) | (rt << 8) | (rd << 4) | funct;
}

insn_t gen_i_type_insn(uint_t opcode, uint_t rs, uint_t rt, uint_t imm) {
    return (opcode << 16) | (rs << 12) | (rt << 8) | (imm & 0xFF);
}

insn_t gen_b_type_insn(uint_t opcode, uint_t rs, uint_t bt_funct, uint_t imm) {
    return (opcode << 16) | (rs << 12) | (bt_funct << 10) | (imm & 0x3FF);
}

insn_t gen_j_type_insn(uint_t opcode, uint_t rs, uint_t imm) {
    return (opcode << 16) | (rs << 12) | (imm & 0xFFF);
}

HASH_MAP_TYPE(str, uint_t) INSN_OPCODES;
HASH_MAP_TYPE(str, uint_t) INSN_FUNCT;
HASH_MAP_TYPE(str, uint_t) INSN_BT_FUNCT;

// ALUInsn := ALUInsnOpcode Reg Reg Reg
// ALUInsnOpcode := ADD | SUB | AND | OR | XOR | SAR | SHL | SHR | SET | CLR

uint_t parse_alu_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, rd, funct;
    rd = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    rt = parse_reg_no(ptr);
    HASH_MAP_GET(INSN_FUNCT, insn_name, funct, str, uint_t, str_equal);
    return gen_r_type_insn(opcode, rs, rt, rd, funct);
}

// ImmALUInsn := ImmALUInsnOpcode Reg Reg Imm
// ImmALUInsnOpcode := ADDI | ANDI | ORI | XORI | SARI | SHLI | SHRI | SETI | CLRI

uint_t parse_imm_alu_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, imm;
    rt = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    imm = eval_const_expr(ctx, ptr) & 0xFF;
    if (is_bit_manip_insn(opcode)) {
        uint_t funct;
        HASH_MAP_GET(INSN_FUNCT, insn_name, funct, str, uint_t, str_equal);
        if (funct == FN_NIL) {
            error("Unrecognized instruction: %s\n", insn_name);
        }

        imm = (imm << 5) | funct;
    }

    return gen_i_type_insn(opcode, rs, rt, imm);
}

// MemoryInsn := MemoryInstOpcode Reg MemAddr
// MemAddr := Variable | Imm\(Reg\)
// MemoryInsnOpcode := ILOAD | ISTORE

uint_t parse_memory_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, imm;
    rt = parse_reg_no(ptr);
    parse_mem_addr(ctx, ptr, &rs, &imm);
    return gen_i_type_insn(opcode, rs, rt, imm);
}

// StackInsn := StackInsnOpcode Reg
// StackInsnOpcode := PUSH | POP

uint_t parse_stack_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, rd, funct;
    rs = rt = rd = 0;
    HASH_MAP_GET(INSN_FUNCT, insn_name, funct, str, uint_t, str_equal);
    *((funct == FN_PUSH) ? &rs : &rd) = parse_reg_no(ptr);  // PUSH & POP share opcodes but not function code.
    return gen_r_type_insn(opcode, rs, rt, rd, funct);
}

// JmpInsn := JmpInsnOpcode JmpTarget
// JmpInsnOpcode := JMP

uint_t parse_jmp_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, imm, bt_funct;
    rs = 0;
    bt_funct = BF_JMP;
    imm = parse_jmp_target(ctx, ptr);
    return gen_b_type_insn(opcode, rs, bt_funct, imm);
}

// JmpRegInsn := JmpInsnOpcode JmpRegTarget
// JmpRegInsnOpcode := LJMP | INVOKE

uint_t parse_jmp_reg_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, imm;
    parse_jmp_reg_target(ctx, ptr, &rs, &imm);
    return gen_j_type_insn(opcode, rs, imm);
}

// BranchInsn := BranchInsnCode Reg JmpRegTarget
// BranchInsnCode := BEQZ | BNEZ | BGTZ | BLTZ

uint_t parse_branch_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, bt_funct, imm;
    HASH_MAP_GET(INSN_BT_FUNCT, insn_name, bt_funct, str, uint_t, str_equal)
    rs = parse_reg_no(ptr);
    imm = parse_jmp_target(ctx, ptr);
    return gen_b_type_insn(opcode, rs, bt_funct, imm);
}

// RetInsn := SimpleInsnOpcode [I]
// RetInsnOpcode := RET

uint_t parse_ret_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    return (opcode << 16) | (at_line_end(ptr) ? 0x1 : 0x0);
}

// CMPInsn := CMPInsnOpcode Reg Reg Cond Reg
// CMPInsnOpcode := CMPU

uint_t parse_cmp_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, rd, funct;
    rd = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    uint_t cond_code = parse_cond_code(ptr);
    rt = parse_reg_no(ptr);
    funct = FN_CMPU_PREFIX | cond_code;
    return gen_r_type_insn(opcode, rs, rt, rd, funct);
}

// CMPIInsn := CMPIInsnOpcode Reg Reg Cond Reg
// CMPIInsnOpcode := CMPIU

uint_t parse_cmpi_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, rt, imm;
    rt = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    uint_t cond_code = parse_cond_code(ptr);
    imm = eval_const_expr(ctx, ptr) & 0xFF;
    opcode = OP_CMPIU_PREFIX | cond_code;
    return gen_i_type_insn(opcode, rs, rt, imm);
}

// IOInsn := IOInsnOpcode Reg IOPort
// IOInsnOpcode := INCSR | OUTCSR

uint_t parse_io_insn(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr) {
    uint_t rs, bt_funct, imm;
    HASH_MAP_GET(INSN_BT_FUNCT, insn_name, bt_funct, str, uint_t, str_equal)
    rs = parse_reg_no(ptr);
    imm = parse_io_port(ctx, ptr);
    return gen_b_type_insn(opcode, rs, bt_funct, imm);
}

/*********** Line Parsers ***********/

// PreprocessorLine := CONST Identifier (Imm | SignedImm)

void parse_preprocessor_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    str const_name = read_current(ptr);
    uint_t const_val = eval_const_expr(ctx, ptr);
    save_constant(ctx, const_name, const_val);
}

// VariableLine := DB Identifier Imm

void parse_variable_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    str var_name = read_current(ptr);
    uint_t var_size = eval_const_expr(ctx, ptr);
    save_pointer(ctx, var_name, allocate_variable(ctx, var_size));
}

// ORGLine := ORG (SRAM | IM) Imm

void parse_org_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    bool org_im = str_equal(peek_current(ptr), "IM");
    if (!org_im && !str_equal(peek_current(ptr), "SRAM")) error("Unrecognized org target: %s\n", peek_current(ptr));
    skip_current(ptr);
    uint_t position = eval_const_expr(ctx, ptr);
    ctx->next_var_ptr = position;
}

// PointerLine := PTR Identifier Expression

void parse_pointer_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    str ptr_name = read_current(ptr);
    uint_t ptr_pos = eval_const_expr(ctx, ptr);
    save_pointer(ctx, ptr_name, ptr_pos);
}

// InsnLine := [Label:] [Insn]
//
// Label := Identifier
// Insn := ALUInsn | ImmALUInsn | SignedImmALUInsn | MemoryInsn
// 	    | StackInsn | JmpInsn | BranchInsn | SimpleInsn | CMPInsn
//  	| CMPIInsn | IOInsn

typedef uint_t (*insn_parser)(uint_t opcode, str insn_name, parse_ctx *ctx, read_head *ptr);

HASH_MAP_TYPE(str, insn_parser) INSN_PARSERS;

void register_insn(str insn_name, insn_parser parser, uint_t opcode, uint_t funct, uint_t bt_funct) {
    HASH_MAP_PUT(INSN_PARSERS, insn_name, parser, str, insn_parser, str_equal)
    HASH_MAP_PUT(INSN_OPCODES, insn_name, opcode, str, uint_t, str_equal)
    HASH_MAP_PUT(INSN_FUNCT, insn_name, funct, str, uint_t, str_equal)
    HASH_MAP_PUT(INSN_BT_FUNCT, insn_name, bt_funct, str, uint_t, str_equal)
}

void register_insns() {
    HASH_MAP_INIT(str, insn_parser, INSN_PARSERS, 64, hash_str, NULL)
    HASH_MAP_INIT(str, uint_t, INSN_OPCODES, 64, hash_str, OP_INVALID)
    HASH_MAP_INIT(str, uint_t, INSN_FUNCT, 64, hash_str, FN_NIL)
    HASH_MAP_INIT(str, uint_t, INSN_BT_FUNCT, 64, hash_str, BF_NIL)

    register_insn("ADD", parse_alu_insn, OP_ADD, FN_ADD, BF_NIL);
    register_insn("SUB", parse_alu_insn, OP_SUB, FN_SUB, BF_NIL);
    register_insn("AND", parse_alu_insn, OP_AND, FN_AND, BF_NIL);
    register_insn("OR", parse_alu_insn, OP_OR, FN_OR, BF_NIL);
    register_insn("XOR", parse_alu_insn, OP_XOR, FN_XOR, BF_NIL);
    register_insn("SAR", parse_alu_insn, OP_SAR, FN_SAR, BF_NIL);
    register_insn("SHL", parse_alu_insn, OP_SHL, FN_SHL, BF_NIL);
    register_insn("SHR", parse_alu_insn, OP_SHR, FN_SHR, BF_NIL);
    register_insn("SET", parse_alu_insn, OP_SET, FN_SET, BF_NIL);
    register_insn("CLR", parse_alu_insn, OP_CLR, FN_CLR, BF_NIL);

    register_insn("ADDI", parse_imm_alu_insn, OP_ADDI, FN_NIL, BF_NIL);
    register_insn("ANDI", parse_imm_alu_insn, OP_ANDI, FN_NIL, BF_NIL);
    register_insn("ORI", parse_imm_alu_insn, OP_ORI, FN_NIL, BF_NIL);
    register_insn("XORI", parse_imm_alu_insn, OP_XORI, FN_NIL, BF_NIL);
    register_insn("SARI", parse_imm_alu_insn, OP_SARI, FN_SAR, BF_NIL);
    register_insn("SHLI", parse_imm_alu_insn, OP_SHLI, FN_SHL, BF_NIL);
    register_insn("SHRI", parse_imm_alu_insn, OP_SHRI, FN_SHR, BF_NIL);
    register_insn("SETI", parse_imm_alu_insn, OP_SETI, FN_SET, BF_NIL);
    register_insn("CLRI", parse_imm_alu_insn, OP_CLRI, FN_CLR, BF_NIL);

    register_insn("ILOAD", parse_memory_insn, OP_ILOAD, FN_NIL, BF_NIL);
    register_insn("ISTORE", parse_memory_insn, OP_ISTORE, FN_NIL, BF_NIL);

    register_insn("PUSH", parse_stack_insn, OP_PUSH, FN_PUSH, BF_NIL);
    register_insn("POP", parse_stack_insn, OP_POP, FN_POP, BF_NIL);

    register_insn("JMP", parse_jmp_insn, OP_JMP, FN_NIL, BF_JMP);

    register_insn("LJMP", parse_jmp_reg_insn, OP_LJMP, FN_NIL, BF_NIL);
    register_insn("INVOKE", parse_jmp_reg_insn, OP_INVOKE, FN_NIL, BF_NIL);

    register_insn("BEQZ", parse_branch_insn, OP_BEQZ, FN_NIL, BF_BEQZ);
    register_insn("BNEZ", parse_branch_insn, OP_BNEZ, FN_NIL, BF_BNEZ);
    register_insn("BGTZ", parse_branch_insn, OP_BGTZ, FN_NIL, BF_BGTZ);
    register_insn("BLTZ", parse_branch_insn, OP_BLTZ, FN_NIL, BF_BLTZ);

    register_insn("RET", parse_ret_insn, OP_RET, FN_NIL, BF_RET);

    register_insn("CMPU", parse_cmp_insn, OP_CMPU, FN_CMPU_PREFIX, BF_NIL);

    register_insn("CMPIU", parse_cmpi_insn, OP_CMPIU_PREFIX, FN_NIL, BF_NIL);

    register_insn("INCSR", parse_io_insn, OP_INCSR, FN_NIL, BF_INCSR);
    register_insn("OUTCSR", parse_io_insn, OP_OUTCSR, FN_NIL, BF_OUTCSR);
}

void register_cond_codes() {
    HASH_MAP_INIT(str, uint_t, COND_CODES, 64, hash_str, COND_INVALID)
    HASH_MAP_PUT(COND_CODES, "EQ", COND_EQ, str, uint_t, str_equal)
    HASH_MAP_PUT(COND_CODES, "NE", COND_NE, str, uint_t, str_equal)
    HASH_MAP_PUT(COND_CODES, "GT", COND_GT, str, uint_t, str_equal)
    HASH_MAP_PUT(COND_CODES, "LT", COND_LT, str, uint_t, str_equal)
}

void put_insn(parse_ctx *ctx, insn_t insn_bin, uint_t *insn_cnt) {
    uint_t addr = ctx->next_insn_ptr;
    if (ctx->im_memmap[addr]) {
        error("IM overlapped @ 0x%03x", addr);
    } else {
        ctx->im_memmap[addr] = true;
    }

    (*insn_cnt)++;
    ctx->insns[ctx->next_insn_ptr++] = insn_bin;
    if (ctx->next_insn_ptr > IM_CAPACITY) {
        error("IM out of bound inserting 0x%05x", insn_bin);
    }
}

void parse_insn_line(parse_ctx *ctx, read_head *ptr, uint_t *insn_cnt) {
    if (cur_token_match_n(ptr, ":", 1)) {
        // Skipping labels as we have already parsed them
        skip_current(ptr);
        skip_current(ptr);
    }

    if (!at_line_end(ptr)) {
        str insn_name = read_current(ptr);
        insn_parser parse_cur_insn;
        uint_t opcode;
        HASH_MAP_GET(INSN_OPCODES, insn_name, opcode, str, uint_t, str_equal);
        if (opcode == OP_INVALID) {
            error("Unrecognized instruction: %s\n", insn_name);
            ctx->next_insn_ptr++;
            return;
        }

        HASH_MAP_GET(INSN_PARSERS, insn_name, parse_cur_insn, str, insn_parser, str_equal)
        uint_t insn_bin = parse_cur_insn(opcode, insn_name, ctx, ptr);
        put_insn(ctx, insn_bin, insn_cnt);
    }
}

/*********** Misc ***********/

void collect_labels(parse_ctx *ctx, read_head *ptr) {
    str *start = ptr->ptr;
    ctx->next_insn_ptr = PROG_ENTRY;
    while (inbounds(ptr)) {
        if (str_equal(peek_current(ptr), "ORG")) {
            parse_org_line(ctx, ptr);
        } else if (!str_equal(peek_current(ptr), "CONST")
                && !str_equal(peek_current(ptr), "DB")
                && !str_equal(peek_current(ptr), "PTR")) {
            if (cur_token_match_n(ptr, ":", 1)) {
                parse_label(ctx, ptr);
            }

            if (!at_line_end(ptr)) {
                ctx->next_insn_ptr++;
            }
        }

        step_to_next_line(ptr);
    }

    ctx->next_insn_ptr = PROG_ENTRY;
    ctx->next_var_ptr = 0;
    ptr->ptr = start;
}

void get_mem_usage(parse_ctx *ctx, uint_t *sram_usage, uint_t *im_usage) {
    *sram_usage = *im_usage = 0;
    for (uint_t i = 0; i < SRAM_CAPACITY; i++) {
        if (ctx->sram_memmap[i]) {
            (*sram_usage)++;
        }
    }

    for (uint_t i = 0; i < IM_CAPACITY; i++) {
        if (ctx->im_memmap[i]) {
            (*im_usage)++;
        }
    }
}

void init_parser() {
    register_insns();
    register_cond_codes();
}

/*********** Main Routine ***********/

// Line := [(InsnLine | PreprocessorLine | VariableLine | ORGLine | PointerLine)][%Comment]
//
// InsnLine := [Label:] [Insn]
// PreprocessorLine := CONST Identifier (Imm | SignedImm)
// VariableLine := DB Identifier Imm
// ORGLine := ORG Imm
// PointerLine := PTR Identifier Expression

void parse(read_head *ptr, uint_t *insn_buf, uint_t *sram_usage, uint_t *im_usage) {
    parse_ctx ctx;
    init_parse_ctx(&ctx, insn_buf);
    collect_labels(&ctx, ptr);
    uint_t insn_cnt = 0;
    while (inbounds(ptr)) {
        if (str_equal(peek_current(ptr), "CONST")) {
            parse_preprocessor_line(&ctx, ptr);
        } else if (str_equal(peek_current(ptr), "DB")) {
            parse_variable_line(&ctx, ptr);
        } else if (str_equal(peek_current(ptr), "ORG")) {
            parse_org_line(&ctx, ptr);
        } else if (str_equal(peek_current(ptr), "PTR")) {
            parse_pointer_line(&ctx, ptr);
        } else {
            parse_insn_line(&ctx, ptr, &insn_cnt);
        }

        step_to_next_line(ptr);
    }

    debug("\nParsed %u instructions\n", insn_cnt);
    get_mem_usage(&ctx, sram_usage, im_usage);
    free_parse_ctx(&ctx);
}
