// BEQZ   : 1110 xxxx 00 xxxxxxxxxx - if (R[rs] == 8'b0)  PC <- (PC + SignExt(imm))[11:0]
// RET    : 1111 0000 00 0000000000 - PC <- (CallStack.Pop() + 1)
// JMP    : 1111 xxxx 01 xxxxxxxxxx - PC <- (PC + SignExt(imm))[11:0]
// LJMP   :	1001 xxxx xxxxxxxxxxxx - PC <- (imm + SignExt(R[rs]))[11:0]
// INVOKE : 1101 xxxx xxxxxxxxxxxx - CallStack.Push(PC); PC <- (SignExt(R[rs]) + imm)[11:0]

`define IRQ_HANDLER 12'hFF0

module npc_id (
    input   wire    [11:0]  prev_pc, 
    input   wire    [19:16] insn_19_to_16, 
    input   wire    [11:0]  insn_11_to_0,
    input   wire            intr, 
    output  wire            call_push,
    output  wire            call_pop, 
    output  wire            iret, 
    output  wire            jump_at_id, 
    output  wire            jump_at_ex, 
    output  wire            bxxz, 
    output  wire    [1:0]   branch_cond, 
    output  wire    [11:0]  next_pc
);

wire [3:0] opcode;
wire [1:0] btype_fn;
wire [11:0] imm12;

assign opcode = insn_19_to_16;
assign btype_fn = insn_11_to_0[11:10];

wire ret;
wire jmp;
wire ljmp;
wire invoke;

assign bxxz = opcode == 4'b1110;
assign ret = (opcode == 4'b1111) & (btype_fn == 2'b00);
assign jmp = (opcode == 4'b1111) & (btype_fn == 2'b01);
assign ljmp = opcode == 4'b1001;
assign invoke = opcode == 4'b1101;
assign imm12 = (ljmp | invoke) ? insn_11_to_0[11:0] : { { 2 { insn_11_to_0[9] } }, insn_11_to_0[9:0] } ;
assign call_push = invoke | intr;
assign call_pop = ret & ~intr;
assign iret = ret & imm12[0];
assign jump_at_id = jmp;
assign jump_at_ex = bxxz | ret | ljmp | invoke; 

wire abs_jmp;
wire offset_eff;

assign abs_jmp = ljmp | invoke | ret;
assign offset_eff = jmp | iret;
assign branch_cond = btype_fn;

wire [11:0] imm_target;
wire [11:0] offset;

assign imm_target = imm12;
assign offset = imm12;

assign next_pc = intr ? `IRQ_HANDLER : ((abs_jmp ? prev_pc : imm_target) + (offset_eff ? 12'b1 : offset));

endmodule
