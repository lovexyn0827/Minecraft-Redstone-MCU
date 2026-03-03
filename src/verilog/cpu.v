module cpu (
    input   wire            clk, rst, 

    output  wire    [11:0]  imem_addr, 
    input   wire    [19:0]  insn, 

    output  wire    [7:0]   sram_addr, 
    output  wire            sram_wrt,  
    inout   wire    [7:0]   sram_dat, 

    output  wire    [9:0]   csr_bus_addr , 
    output  wire            csr_bus_wrt,
    input   wire            csr_bus_intr,  
    inout   wire    [7:0]   csr_bus_dat
);

// ================ IF stage ================
// NPC stage is not necessary as NextPC calculation & Insn fetch can go in parallel

reg [11:0] pc;
wire [11:0] next_pc;

always @(posedge clk) begin
    pc <= rst ? 12'h010 : next_pc;
end

wire handling_intr, iret, jump_intr;
reg intr, suppress_intr;

assign handling_intr = intr & ~iret;
assign jump_intr = intr & ~suppress_intr;

always @(posedge clk) begin
    intr <= rst ? 1'b0 : csr_bus_intr;
    suppress_intr <= rst ? 1'b0 : handling_intr;
end

wire call_push, call_pop;
wire [11:0] call_stack_out;

stack #( .WLEN(12), .DEPTH(8), .PTR_LEN(3) ) call_stack (
    .din(pc), 
    .dout(call_stack_out), 
    .clk(clk), 
    .rst(rst), 
    .push(call_push), 
    .pop(call_pop)
);

wire [3:0] rs_NPC;
wire [7:0] rs_val_NPC;
wire bypass_ID2IF;
wire bypass_EX2IF;
wire bypass_MEM2IF;
wire bypass_WB2IF;

npc npc (
    .prev_pc(pc), 
    .insn(insn), 
    .rs_val(rs_val_NPC), 
    .call_stack_out(call_stack_out), 
    .intr(intr), 
    .rs(rs_NPC), 
    .call_push(call_push), 
    .call_pop(call_pop), 
    .iret(iret), 
    .next_pc(next_pc)
);

assign imem_addr = pc;

// ================ ID stage ================

// Pipelining latches
reg [19:0] insn_ID;
reg intr_ID;

always @(posedge clk) begin
    insn_ID <= insn;
    intr_ID <= jump_intr;
end

wire [3:0] alu_func;
wire operand_push, operand_pop;
wire [9:0] imm10;
wire [3:0] rf_ri1, rf_ri2, rf_wi;
wire rf_wrt;
wire sram2rf;
wire imm2alu;
wire csr2rf;
wire csr_wrt;

wire [7:0] alu_out;
wire [7:0] rf_rd1, rf_rd2;

cu cu (
    .insn(insn_ID), 
    .intr(intr_ID), 
    .alu_func(alu_func), 
    .sram_wrt(sram_wrt), 
    .imm10_out(imm10), 
    .operand_push(operand_push), 
    .operand_pop(operand_pop),
    .rf_ri1(rf_ri1), 
    .rf_ri2(rf_ri2), 
    .rf_wi(rf_wi), 
    .rf_wrt(rf_wrt), 
    .sram2rf(sram2rf), 
    .imm2alu(imm2alu), 
    .csr2rf(csr2rf),
    .csr_wrt(csr_wrt)
);

wire [7:0] rf_wdat;

rf rf (
    .clk(clk), 
    .rst(rst), 
    .wrt(rf_wrt), 
    .ri1(rf_ri1), 
    .ri2(rf_ri2), 
    .wi(rf_wi), 
    .wd(rf_wdat), 
    .rd1(rf_rd1), 
    .rd2(rf_rd2)
);

// ================ EX stage ================

// Pipelining latches
reg imm2alu_EX;
reg [3:0] alu_func_EX;

// Unused, only for progation
reg [7:0] rf_rd1_EX;
reg [7:0] rf_rd2_EX;
reg sram_wrt_EX;
reg operand_pop_EX;
reg operand_push_EX;
reg [9:0] imm10_EX;
reg csr_bus_wrt_EX;
reg csr2rf_EX;
reg sram2rf_EX;

always @(posedge clk) begin
    // All signals are fresh
    imm2alu_EX <= imm2alu;
    alu_func_EX <= alu_func;
    rf_rd1_EX <= rf_rd1;
    rf_rd2_EX <= rf_rd2;
    sram_wrt_EX <= sram_wrt;
    operand_pop_EX <= operand_pop;
    operand_push_EX <= operand_push;
    imm10_EX <= imm10;
    csr_bus_wrt_EX <= csr_wrt;
    csr2rf_EX <= csr2rf;
    sram2rf_EX <= sram2rf;
end

wire [7:0] alu_in1, alu_in2;
assign alu_in1 = rf_rd1_EX;
assign alu_in2 = imm2alu_EX ? imm10_EX[7:0] : rf_rd2_EX;

alu alu (
    .a(alu_in1), 
    .b(alu_in2), 
    .z(alu_out), 
    .func(alu_func_EX)
);

// ================ MEM stage ================

// Pipelining latches
reg [7:0] alu_out_MEM;
reg [7:0] rf_rd1_MEM;
reg [7:0] rf_rd2_MEM;
reg sram_wrt_MEM;
reg operand_pop_MEM;
reg operand_push_MEM;
reg [9:0] imm10_MEM;
reg csr_bus_wrt_MEM;

// Unused, only for progation
reg sram2rf_MEM;
reg csr2rf_MEM;

always @(posedge clk) begin
    // Fresh: alu_out
    alu_out_MEM <= alu_out;
    rf_rd1_MEM <= rf_rd1_EX;
    rf_rd2_MEM <= rf_rd2_EX;
    sram_wrt_MEM <= sram_wrt_EX;
    operand_pop_MEM <= operand_pop_EX;
    operand_push_MEM <= operand_push_EX;
    imm10_MEM <= imm10_EX;
    csr_bus_wrt_MEM <= csr_bus_wrt_EX;
    sram_wrt_MEM <= sram2rf_EX;
    csr2rf_MEM <= csr2rf_EX;
    sram2rf_MEM <= sram2rf_EX;
    csr2rf_MEM <= csr2rf_EX;
end

wire [7:0] operand_stack_out;

stack #( .WLEN(8), .DEPTH(32), .PTR_LEN(5) ) operand_stack (
    .din(rf_rd1_MEM), 
    .dout(operand_stack_out), 
    .clk(clk), 
    .rst(rst), 
    .push(operand_push_MEM), 
    .pop(operand_pop_MEM)
);

assign sram_addr = alu_out_MEM;
assign sram_dat = sram_wrt_MEM ? rf_rd2_MEM : 8'bZZZZZZZZ;

assign csr_bus_dat = csr_bus_wrt_MEM ? rf_rd1_MEM : 8'bZZZZZZZZ;
assign csr_bus_addr = imm10_MEM;
assign csr_bus_wrt = csr_bus_wrt_MEM;

// ================ WB stage ================

// Pipelining latches, all of which are used
reg [7:0] alu_out_WB;
reg sram2rf_WB;
reg [7:0] sram_dat_WB;
reg operand_pop_WB;
reg [7:0] operand_stack_out_WB;
reg csr2rf_WB;
reg [7:0] csr_bus_dat_WB;

always @(posedge clk) begin
    // Fresh: CSRDat, SRAMDat, OSOut
    alu_out_WB <= alu_out_WB;
    sram2rf_WB <= sram2rf_MEM;
    sram_dat_WB <= sram_dat;
    operand_pop_WB <= operand_pop_MEM;
    operand_stack_out_WB <= operand_stack_out;
    csr_bus_dat_WB <= csr_bus_dat;
    csr2rf_WB <= csr2rf_MEM;
end

assign rf_wdat = sram2rf_WB ? sram_dat_WB : (operand_pop_WB ? operand_stack_out_WB : (csr2rf_WB ? csr_bus_dat_WB : alu_out_WB));

endmodule
