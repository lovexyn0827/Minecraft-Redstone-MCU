// BEQZ   : 1110 xxxx 00 xxxxxxxxxx - if (R[rs] == 8'b0)  PC <- (PC + SignExt(imm))[11:0]
// RET    : 1111 0000 00 0000000000 - PC <- (CallStack.Pop() + 1)
// JMP    : 1111 xxxx 01 xxxxxxxxxx - PC <- (PC + SignExt(imm))[11:0]
// LJMP   :	1001 xxxx xxxxxxxxxxxx - PC <- (imm + SignExt(R[rs]))[11:0]
// INVOKE : 1101 xxxx xxxxxxxxxxxx - CallStack.Push(PC); PC <- (SignExt(R[rs]) + imm)[11:0]

`define IRQ_HANDLER 12'hFF0

module npc (
    input   wire    [11:0]  prev_pc, 
    input   wire    [19:0]  insn, 
    input   wire    [7:0]   rs_val, 
    input   wire    [11:0]  call_stack_out, 
    input   wire            intr, 
    output  wire    [3:0]   rs, 
    output  wire            call_push,
    output  wire            call_pop, 
    output  wire            iret, 
    output  wire    [11:0]  next_pc
);

wire [3:0] opcode;
wire [1:0] btype_fn;
wire [11:0] imm12;

assign opcode = insn[19:16];
assign btype_fn = insn[11:10];

wire bxxz;
wire ret;
wire jmp;
wire ljmp;
wire invoke;

assign bxxz = opcode == 4'b1110;
assign ret = (opcode == 4'b1111) & (btype_fn == 2'b00);
assign jmp = (opcode == 4'b1111) & (btype_fn == 2'b01);
assign ljmp = opcode == 4'b1001;
assign invoke = opcode == 4'b1101;
assign imm12 = (ljmp | invoke) ? insn[11:0] : { { 2 { insn[9] } }, insn[9:0] } ;
assign call_push = invoke | intr;
assign call_pop = ret & ~intr;
assign iret = ret & imm12[0];
assign rs = insn[15:12];

wire should_branch;

assign should_branch = { rs_val == 8'b0, rs_val != 8'b0, ~rs_val[7], rs_val[7] } [btype_fn];

wire abs_jmp;
wire offset_eff;

assign abs_jmp = ljmp | invoke | ret;
assign offset_eff = (bxxz & should_branch) | jmp | iret;

wire [11:0] imm_target;
wire [11:0] offset;

assign imm_target = ret ? call_stack_out : imm12;
assign offset = (ljmp | invoke) ? { { 4 { rs_val[7] } }, rs_val } : imm12;

assign next_pc = intr ? `IRQ_HANDLER : ((abs_jmp ? prev_pc : imm_target) + (offset_eff ? 12'b1 : offset));

endmodule
