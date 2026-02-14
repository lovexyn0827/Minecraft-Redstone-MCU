`define ALU_FN_ADD  (4'b0000)
`define ALU_FN_SUB  (4'b0001)

`define NPC_OFFSET  (2'b00)
`define NPC_SEQ     (2'b01)
`define NPC_IMM     (2'b10)
`define NPC_RET     (2'b11)

module cu (
    input   wire    [19:0]  insn, 
    input   wire    [7:0]   rs_val, 
    output  wire    [3:0]   alu_func, 
    output  wire            rf_wrt, 
    output  wire    [3:0]   rf_ri1, rf_ri2, rf_wi, 
    output  wire            operand_push, operand_pop, 
    output  wire            call_push, call_pop, 
    output  wire    [1:0]   npc_mode, 
    output  wire    [11:0]  imm12_out, 
    output  wire            sram2rf, sram_wrt, 
    output  wire            imm2alu, 
    output  wire            csr2rf, csr_wrt, 
    output  wire            jmp_with_reg
);

wire [3:0] opcode;
wire [3:0] rs, rt, rd;
wire [2:0] imm3;
wire [7:0] imm8;
wire [9:0] imm10;
wire [11:0] imm12;
wire [3:0] func;
wire [1:0] btype_fn;

assign opcode = insn[19:16];
assign rs = insn[15:12];
assign rt = insn[11:8];
assign rd = insn[7:4];
assign func = insn[3:0];
assign imm3 = insn[7:5];
assign imm8 = insn[7:0];
assign imm10 = insn[9:0];
assign imm12 = insn[11:0];
assign btype_fn = insn[11:10];

// ADD   : 0000 xxxx xxxx xxxx 0000 - R[rd] <- (R[rs] + R[rt])[7:0]
// SUB   : 0000 xxxx xxxx xxxx 0001 - R[rd] <- (R[rs] - R[rt])[7:0]
// AND   : 0000 xxxx xxxx xxxx 0010 - R[rd] <- R[rs] & R[rt]
// OR    : 0000 xxxx xxxx xxxx 0011 - R[rd] <- R[rs] | R[rt]
// XOR   : 0000 xxxx xxxx xxxx 0100 - R[rd] <- R[rs] ^ R[rt]
// SAR   : 0000 xxxx xxxx xxxx 0101 - R[rd] <- (R[rs] >> R[rt][2:0])[7:0]
// SHL   : 0000 xxxx xxxx xxxx 0110 - R[rd] <- (R[rs] << R[rt][2:0])[7:0]
// SHR   : 0000 xxxx xxxx xxxx 0111 - R[rd] <- (R[rs] >>> R[rt][2:0])[7:0]
// SET   : 0000 xxxx xxxx xxxx 1000 - R[rd] <- (R[rs] & ~(1 << R[rt]))[7:0]
// CLR   : 0000 xxxx xxxx xxxx 1001 - R[rd] <- (R[rs] | (1 << R[rt]))[7:0]
// PUSH  : 0000 xxxx 0000 0000 1010 - OperandStack.Push(R[rs])
// POP   : 0000 0000 0000 xxxx 1011 - R[rd] <- OperandStack.Pop()
// CMPU  : 0000 xxxx xxxx xxxx 11md - @ = { =, !=, >, < }[md]; R[rd] <- R[rs] @ R[rt] ? 1 : 0

wire rtype;
wire rtype_rf_wrt;
wire [3:0] rtype_rf_ri1, rtype_rf_ri2, rtype_rf_wi;
wire [3:0] rtype_alu_func;

assign rtype = opcode == 4'b0;
assign operand_push = rtype & func == 4'b1010;
assign operand_pop = rtype & func == 4'b1011;
assign rtype_rf_ri1 = rs;
assign rtype_rf_ri2 = rt;
assign rtype_rf_wi = rd;
assign rtype_alu_func = func;

assign rtype_rf_wrt = 1'b1; // rf_wrt=1 when pushing is fine since we are writing to immutable r0.

// SARI  : 0001 xxxx xxxx xxx00101 - R[rt] <- (R[rs] >> imm[7:5])[7:0]
// SHLI  : 0001 xxxx xxxx xxx00110 - R[rt] <- (R[rs] << imm[7:5])[7:0]
// SHRI  : 0001 xxxx xxxx xxx00111 - R[rt] <- (R[rs] >>> imm[7:5])[7:0]
// SETI  : 0001 xxxx xxxx xxx01000 - R[rt] <- (R[rs] & ~(1 << imm[7:5]))[7:0] 
// CLRI  : 0001 xxxx xxxx xxx01001 - R[rt] <- (R[rs] | (1 << imm[7:5]))[7:0]
// ILOAD : 0010 xxxx xxxx xxxxxxxx - R[rt] <- Mem[(R[rs] + imm)[7:0]]
// ISTORE: 0011 xxxx xxxx xxxxxxxx - Mem[(R[rs] + imm)[7:0]] <- R[rt]
// CMPIU : 01md xxxx xxxx xxxxxxxx - @ = { =, !=, >, < }[md]; R[rt] <- R[rs] @ imm ? 1 : 0
// ADDI  : 1000 xxxx xxxx xxxxxxxx - R[rt] <- (R[rs] + imm)[7:0]
// ANDI  : 1010 xxxx xxxx xxxxxxxx - R[rt] <- R[rs] & imm
// ORI   : 1011 xxxx xxxx xxxxxxxx - R[rt] <- R[rs] | imm
// XORI  : 1100 xxxx xxxx xxxxxxxx - R[rt] <- R[rs] ^ imm

wire itype;
wire [3:0] itype_alu_func;
wire imm_bit_man_insn;
wire mem_acc_insn;
wire itype_rf_wrt;
wire [3:0] itype_rf_ri1, itype_rf_ri2, itype_rf_wi;
wire [11:0] itype_imm_out;

assign itype = (~opcode[3] & (opcode[2] | opcode [1] | opcode[0])) 
    | (~opcode[2] & opcode[1]) | (opcode[3] & ~opcode[1] & ~opcode[0]);
assign imm_bit_man_insn = opcode == 4'b0001;
assign mem_acc_insn = opcode[3:1] == 3'b001;
assign sram_wrt = mem_acc_insn & opcode[0];
assign itype_alu_func = mem_acc_insn ? `ALU_FN_ADD : (imm_bit_man_insn ? func : { ~opcode[3], opcode[2:0] });
assign itype_rf_wrt = ~sram_wrt; // All insns write to RF except ISTORE
assign sram2rf = opcode == 4'b0010;
assign itype_rf_ri1 = rs;
assign itype_rf_ri2 = rt;
assign itype_rf_wi = rt;
// Unified signed ext. is fine since imm12 -> imm8 when entering the ALU
assign itype_imm_out = imm_bit_man_insn ? { 9'b1, imm3 } : { { 4 { imm8[7] } }, imm8 };


// BEQZ  : 1110 xxxx 00 xxxxxxxxxx - if (R[rs] == 8'b0)  PC <- (PC + SignExt(imm))[11:0]
// BNEZ  : 1110 xxxx 01 xxxxxxxxxx - if (R[rs] != 8'b0)  PC <- (PC + SignExt(imm))[11:0]
// BGTZ  : 1110 xxxx 10 xxxxxxxxxx - if (R[rs] >= 8'h7F) PC <- (PC + SignExt(imm))[11:0]
// BLTZ  : 1110 xxxx 11 xxxxxxxxxx - if (R[rs] <= 8'h80) PC <- (PC + SignExt(imm))[11:0]
// RET   : 1111 0000 00 0000000000 - PC <- (CallStack.Pop() + 1)
// JMP   : 1111 xxxx 01 xxxxxxxxxx - PC <- (PC + SignExt(imm))[11:0]
// INCSR : 1111 xxxx 10 xxxxxxxxxx - CSR[imm] <- R[rs]
// OUTCSR: 1111 xxxx 11 xxxxxxxxxx - r[rs] <- CSR[imm]

wire btype;
wire [3:0] btype_alu_func;
wire branch_insn;
wire should_branch;
wire [1:0] btype_npc_mode;
wire ret, jmp;
wire [3:0] branch_cond;
wire btype_rf_wrt;
wire [3:0] btype_rf_ri1, btype_rf_wi;
wire [11:0] btype_imm_out;


assign btype = opcode[3] & opcode[2] & opcode[1];
assign branch_insn = opcode == 4'b1110;
assign btype_alu_func = branch_insn ? { 2'b11, btype_fn } : `ALU_FN_ADD;
assign ret = (opcode == 4'b1111) & (btype_fn == 2'b00);
assign jmp = (opcode == 4'b1111) & (btype_fn == 2'b01);
assign call_pop = ret;
// We won't use the ALU to give branching conditions - saving time and an external controlling signal
assign branch_cond = { ~rs_val[7], rs_val[7], rs_val != 8'b0, rs_val == 8'b0 };
assign should_branch = branch_insn ? branch_cond[btype_fn] : 1'b1;  // We don't suppress jumps IF ANY
// Todo: bit-level hacks
assign btype_npc_mode = branch_insn ? 
    (should_branch ? `NPC_OFFSET : `NPC_SEQ) : (jmp ? `NPC_OFFSET : (ret ? `NPC_RET : `NPC_SEQ));
assign btype_rf_wrt = (opcode == 4'b1111) & (btype_fn == 2'b10);
assign btype_rf_ri1 = rs;
assign btype_rf_wi = rs;
assign btype_imm_out = { { 2 { imm10[9] } }, imm10 };
assign csr2rf = (opcode == 4'b1111) & (btype_fn == 2'b10);
assign csr_wrt = (opcode == 4'b1111) & (btype_fn == 2'b11);

// LJMP  : 1001 xxxx xxxxxxxxxxxx - PC <- (imm + SignExt(R[rs]))[11:0]
// INVOKE: 1101 xxxx xxxxxxxxxxxx - CallStack.Push(PC); PC <- (SignExt(R[rs]) + imm)[11:0]

wire jtype;
wire [1:0] jtype_npc_mode;
wire [11:0] jtype_imm_out;

assign jtype = opcode[3] & ~opcode[1] & opcode[0];
assign jtype_npc_mode = `NPC_IMM;
assign call_push = opcode == 4'b1101;
assign jtype_imm_out = imm12;
assign jmp_with_reg = jtype;

assign rf_wrt = rtype ? rtype_rf_wrt : (itype ? itype_rf_wrt : btype_rf_wrt);   // J-Type insns don't write to RF
assign rf_ri1 = rtype ? rtype_rf_ri1 : (itype ? itype_rf_ri1 : btype_rf_ri1);   // J-Type insns don't operate on RF
assign rf_ri2 = rtype ? rtype_rf_ri2 : itype_rf_ri2;   // Only R-Type insns & ISTORE read two registers at once.
assign rf_wi = rtype ? rtype_rf_wi : (itype ? itype_rf_wi : btype_rf_wi);       // J-Type insns don't operate on RF

assign imm2alu = itype; // Only I-Type insns send immediates to the ALU (not NPC).
assign imm12_out = itype ? itype_imm_out : (btype ? btype_imm_out : jtype_imm_out);

assign alu_func = rtype ? rtype_alu_func : (itype ? itype_alu_func : btype_alu_func); // J-Type insns don't use ALU

assign npc_mode = (btype | jtype) ? (btype ? btype_npc_mode : jtype_npc_mode) : `NPC_SEQ;

endmodule
