#include "parser.h"
#include "string.h"

#include "const_expression.h"

void error() {
    // TODO
}

uint32_t hash_str(const str in) {

}

bool inbounds(read_head *ptr) {
    return ptr->ptr < ptr->max;
}

bool inbounds_after_n(read_head *ptr, uint32_t offset) {
    return (ptr->ptr + offset) < ptr->max;
}

void assert_inbounds(read_head *ptr) {
    if (!inbounds(ptr)) {
        error();
    }
}

const str peek_current(read_head *ptr) {
    return inbounds(ptr) ? *(ptr->ptr) : NULL;
}

const str peek_current_plus_n(read_head *ptr, uint32_t offset) {
    return inbounds_after_n(ptr, offset) ? *(ptr->ptr + offset) : NULL;
}

const str read_current(read_head *ptr) {
    return inbounds(ptr) ? *((ptr->ptr)++) : NULL;
}

bool cur_token_match(read_head *ptr, const str against) {
    const str cur_token = peek_current(ptr);
    return cur_token != NULL && strcmp(cur_token, against) == 0;
}

bool cur_token_match_n(read_head *ptr, const str against, uint32_t n) {
    const str cur_token = peek_current_plus_n(ptr, n);
    return cur_token != NULL && strcmp(cur_token, against) == 0;
}

bool at_line_end(read_head *ptr) {
    return !inbounds(ptr) || *peek_current(ptr) == '\n';
}

void step_to_next_line(read_head *ptr) {
    while (!at_line_end(ptr)) {
        skip_current(ptr);
    }

    while (at_line_end(ptr)) {
        skip_current(ptr);
    }
}

void skip_current(read_head *ptr) {
    if (inbounds(ptr)) (ptr->ptr)++;
}

void save_constant(parse_ctx *ctx, const uint8_t *name, uint32_t value) {
    // TODO
}

void save_pointer(parse_ctx *ctx, const uint8_t *name, uint32_t value) {
    // TODO
}

void save_label(parse_ctx *ctx, const uint8_t *name, uint32_t pos) {
    // TODO
}

uint32_t get_constant(parse_ctx *ctx, const uint8_t *name) {
    // TODO
}

uint32_t get_pointer(parse_ctx *ctx, const uint8_t *name) {
    // TODO
}

uint32_t get_label(parse_ctx *ctx, const uint8_t *name) {
    // TODO
}

uint32_t allocate_variable(parse_ctx *ctx, uint32_t size) {
    // TODO Clamp space
    uint32_t pos = ctx->next_var_ptr;
    ctx->next_var_ptr += size;
    return pos;
}

// PreprocessorLine := CONST Identifier (Imm | SignedImm)

void parse_preprocessor_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    const str const_name = read_current(ptr);
    uint32_t const_val = eval_const_expr(ctx, ptr);
    save_constant(ctx, const_name, const_val);
}

// VariableLine := DB Identifier Imm

void parse_variable_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    const str var_name = read_current(ptr);
    uint32_t var_size = eval_const_expr(ctx, ptr);
    save_pointer(ctx, var_name, allocate_variable(ctx, var_size));
}

// ORGLine := ORG Imm

void parse_org_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    uint32_t position = eval_const_expr(ctx, ptr);
    ctx->next_var_ptr = position;
}

// PointerLine := PTR Identifier Expression

void parse_pointer_line(parse_ctx *ctx, read_head *ptr) {
    skip_current(ptr);
    const str ptr_name = read_current(ptr);
    uint32_t ptr_pos = eval_const_expr(ctx, ptr);
    save_pointer(ctx, ptr_name, ptr_pos);
}

void parse_label(parse_ctx *ctx, read_head *ptr) {
    const str label_name = read_current(ptr);
    save_label(ctx, label_name, ctx->next_insn_ptr);
    skip_current(ptr);
}

bool at_reg_no(read_head *ptr) {
    if (!inbounds(ptr)) {
        return false;
    }

    const str regno_str = read_current(ptr);
    if (strlen(regno_str) < 2 || strlen(regno_str) > 3) {
        return false;
    }

    if (regno_str[0] | 0x20 != 'r') {
        return false;
    }

    uint32_t regno = 0;
    for (uint32_t i = 1; i < strlen(regno_str); i++) {
        if (!isdigit(regno_str[i])) {
            return false;
        }

        regno *= 10;
        regno += regno_str[i] - '0';
    }

    if (regno > 15) {
        return false;
    }

    return true;
}

uint32_t parse_reg_no(read_head *ptr) {
    assert_inbounds(ptr);
    const str regno_str = read_current(ptr);
    if (strlen(regno_str) < 2 || strlen(regno_str) > 3) {
        error();
    }

    if (regno_str[0] | 0x20 != 'r') {
        error();
    }

    int regno = 0;
    for (uint32_t i = 1; i < strlen(regno_str); i++) {
        if (!isdigit(regno_str[i])) {
            error();
        }

        regno *= 10;
        regno += regno_str[i] - '0';
    }

    if (regno > 15) {
        error();
    }

    return regno;
}

MAP_TYPE(str, uint32_t) INSN_OPCODES;
MAP_TYPE(str, uint32_t) INSN_FUNCT;

// ALUInsn := ALUInsnOpcode Reg, Reg, Reg

uint32_t parse_alu_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {
    uint32_t rs, rt, rd, funct;
    rd = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    rt = parse_reg_no(ptr);
    MAP_GET(INSN_FUNCT, insn_name, funct, str, uint32_t);
    return (opcode << 16) | (rs << 12) | (rd << 8) | (rt << 4) | funct;
}

// ImmALUInsn := ImmALUInsnOpcode Reg, Reg, Imm

bool is_bit_manip_insn(uint32_t opcode) {
    return opcode == OP_BITMANIP;
}

uint32_t parse_imm_alu_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {
    uint32_t rs, rt, imm;
    rt = parse_reg_no(ptr);
    rs = parse_reg_no(ptr);
    uint32_t imm = eval_const_expr(ctx, ptr) & 0xFF;
    if (is_bit_manip_insn(opcode)) imm = (imm << 5) | MAP_GET(INSN_FUNCT, insn_name, funct, str, uint32_t);
    return (opcode << 16) | (rs << 12) | (rt << 8) | imm;
}

// MemoryInsn := MemoryInstOpcode Reg, MemAddr
// MemAddr := Variable | Imm\(Reg\)

void parse_mem_addr(parse_ctx *ctx, read_head *ptr, uint32 *rt, uint32 *imm, uint32_t base) {
    if (at_reg_no(ptr)) {
        parse_relative_addr(ptr, rt, imm);
    } else {
        return eval_const_expr(ptr) - base;
    }
}

void parse_relative_addr(read_head *ptr, uint32 *rt, uint32 *imm) {
    *rt = parse_reg_no();
    skip_current(ptr);  // skips '('
    *imm = eval_const_expr(ctx, ptr) & 0xFF;
    skip_current(ptr);  // skips ')'
}

uint32_t parse_imm_alu_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {
    uint32_t rs, rt, imm;
    rs = parse_reg_no(ptr);
    parse_mem_addr(ctx, ptr, &rt, &imm, 0);
    return (opcode << 16) | (rs << 12) | (rt << 8) | (imm & 0xFF);
}

// StackInsn := StackInsnOpcode Reg

uint32_t parse_stack_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {
    uint32_t rs, rt, rd, imm;
    *(opcode == OP_PUSH ? &rs : &rd) = parse_reg_no;
    MAP_GET(INSN_FUNCT, insn_name, funct, str, uint32_t);
    return (opcode << 16) | (rs << 12) | (rd << 4) | funct;
}

// JmpInsn := JmpInsnOpcode JmpTarget
// JmpTarget := Label | Imm\(Reg\)

uint32_t parse_jmp_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {
    uint32_t rs, imm;
    *(opcode == OP_PUSH ? &rs : &rd) = parse_reg_no;
    MAP_GET(INSN_FUNCT, insn_name, funct, str, uint32_t);
    return (opcode << 16) | (rs << 12) | (rd << 4) | funct;
}

// JmpDirectInst := JmpDirectInsnOpcode JmpDirectTarget
// JmpDirectTarget := Label | Imm

void parse_jmp_target(parse_ctx *ctx, read_head *ptr, uint32 *rs, uint32 *imm) {
    if (at_reg_no(ptr)) {
        parse_relative_addr(ptr, rs, imm, ctx->next_insn_ptr);
    } else {
        return eval_const_expr(ptr) - base;
    }
}

uint32_t parse_jmp_direct_insn(uint32_t opcode, parse_ctx *ctx, read_head *ptr) {

}

typedef uint32_t (*insn_parser)(uint32_t opcode, parse_ctx *ctx, read_head *ptr);

MAP_TYPE(str, insn_parser) INSN_PARSERS;

void init_insn_parser_map() {
    MAP_INIT(str, insn_parser, INSN_PARSERS, 64, hash_str, error)
}

// InsnLine := [Label:] [Insn]
//
// Label := Identifier
// Insn := ALUInsn | ImmALUInsn | SignedImmALUInsn | MemoryInsn
// 	    | StackInsn | JmpInsn | BranchInsn | SimpleInsn | CMPInsn
//  	| CMPIInsn | IOInsn
// ALUInsn := ALUInsnOpcode Reg, Reg, Reg
// ImmALUInsn := ImmALUInsnOpcode Reg, Reg, Imm
// MemoryInsn := MemoryInstOpcode Reg, MemAddr
// StackInsn := StackInsnOpcode Reg
// JmpInsn := JmpInsnOpcode JmpTarget
// BranchInsn := BranchInsnCode Reg, BranchTarget
// SimpleInsn := SimpleInsnOpcode
// CMPInsn := CMP Reg, Reg Cond Reg
// CMPIInsn := CMPI Reg, Reg Cond SignedImm
// IOInst := IOInsnOpcode Reg, IOPort

void parse_insn_line(parse_ctx *ctx, read_head *ptr) {
    if (cur_token_match_n(ptr, ":", 1)) {
        parse_label(ctx, ptr);
    }

    if (!at_line_end(ptr)) {
        const str insn_name = read_current(ptr);
        insn_parser parse_cur_insn;
        uint32_t opcode;
        MAP_GET(INSN_OPCODES, insn_name, opcode, str, uint32_t);
        MAP_GET(INSN_PARSERS, insn_name, parse_cur_insn, const str, insn_parser)
        uint32_t insn_bin = parse_cur_insn(opcode, ctx, ptr);
        ctx.insns[ctx.next_insn_ptr++] = insn_bin;
    }
}

// Line := [(InsnLine | PreprocessorLine | VariableLine | ORGLine | PointerLine)][%Comment]
//
// InsnLine := [Label:] [Insn]
// PreprocessorLine := CONST Identifier (Imm | SignedImm)
// VariableLine := DB Identifier Imm
// ORGLine := ORG Imm
// PointerLine := PTR Identifier Expression

void init_parse_ctx(parse_ctx *ctx) {
    ctx->next_var_ptr = 0x00;
    ctx->next_insn_ptr = 0x010;
}

void parse0(read_head *ptr) {
    parse_ctx ctx;
    init_parse_ctx(&ctx);
    while (inbounds(ptr)) {
        if (strcmp(peek_current(ptr), "CONST") == 0) {
            parse_preprocessor_line(&ctx, ptr);
        } else if (strcmp(peek_current(ptr), "DB") == 0) {
            parse_variable_line(&ctx, ptr);
        } else if (strcmp(peek_current(ptr), "ORG") == 0) {
            parse_org_line(&ctx, ptr);
        } else if (strcmp(peek_current(ptr), "PTR") == 0) {
            parse_pointer_line(&ctx, ptr);
        } else {
            parse_insn_line(&ctx, ptr);
        }

        step_to_next_line(ptr);
    }
}
