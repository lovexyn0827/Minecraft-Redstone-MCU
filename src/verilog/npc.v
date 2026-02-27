// BEQZ   : 1110 xxxx 00 xxxxxxxxxx - if (R[rs] == 8'b0)  PC <- (PC + SignExt(imm))[11:0]
// RET    : 1111 0000 00 0000000000 - PC <- (CallStack.Pop() + 1)
// JMP    : 1111 xxxx 01 xxxxxxxxxx - PC <- (PC + SignExt(imm))[11:0]
// LJMP   :	1001 xxxx xxxxxxxxxxxx - PC <- (imm + SignExt(R[rs]))[11:0]
// INVOKE : 1101 xxxx xxxxxxxxxxxx - CallStack.Push(PC); PC <- (SignExt(R[rs]) + imm)[11:0]

`define IRQ_HANDLER 12'hC00

module npc (
    input   wire    [11:0]  prev_pc, 
    input   wire    [11:0]  imm_target, 
    input   wire    [11:0]  offset,     // R[rs] if jumping with an register offset, or imm12 otherwise
    input   wire    [1:0]   mode,       // 00: OFFSET, 01: SEQUENTIAL, 10: IMMEDIATE, 11: RETURN
    output  wire    [11:0]  next_pc
);

// 00: PC <- PC + offset
// 01: PC <- PC + 1
// 10: PC <- IMM + offset
// 11: PC <- IMM + 1

assign next_pc = (mode[1] ? imm_target : prev_pc) + (mode[0] ? 12'b1 : offset);

endmodule
