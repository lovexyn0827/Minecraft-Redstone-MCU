`define NOP (20'h00000)

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

// We simply increasement PC naively, jumps will be handled @ the ID or EX stage
assign next_pc = next_pc_from_EX_eff ? next_pc_from_EX : (next_pc_from_ID_eff ? next_pc_from_ID : pc + 12'b1);

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

assign imem_addr = pc;

// ================ ID stage ================

// Pipelining latches
reg [19:0] insn_ID;
reg intr_ID;

// Unused
reg [11:0] pc_ID;

always @(posedge clk) begin
    insn_ID <= next_pc_from_ID_eff ? `NOP : insn;;
    intr_ID <= jump_intr;
    pc_ID <= pc;
end

wire call_push, call_pop;
wire bxxz;
wire [1:0] branch_cond;

// We determine when to generate branch targets at ID - and targets themselves at ID or EX
// So that we are not referencing R[rs] or the call stack at this moment
wire next_pc_from_ID_eff;
wire [11:0] next_pc_from_ID;
wire next_pc_from_EX_eff_ID;

wire [19:0] insn_filtered_ID;

npc_id npc_id (
    .prev_pc(pc), 
    .insn_19_to_16(insn_ID[19:16]), 
    .insn_11_to_0(insn_ID[11:0]), 
    .intr(intr), 
    .call_push(call_push), 
    .call_pop(call_pop), 
    .iret(iret), 
    .jump_at_id(next_pc_from_ID_eff), 
    .jump_at_ex(next_pc_from_EX_eff_ID), 
    .bxxz(bxxz), 
    .branch_cond(branch_cond), 
    .next_pc(next_pc_from_ID)
);

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

rf rf (
    .clk(clk), 
    .rst(rst), 
    .wrt(rf_wrt_WB), 
    .ri1(rf_ri1), 
    .ri2(rf_ri2), 
    .wi(rf_wi_WB), 
    .wd(rf_wdat_WB), 
    .rd1(rf_rd1), 
    .rd2(rf_rd2)
);

// ================ EX stage ================

// Pipelining latches
reg imm2alu_EX;
reg [3:0] alu_func_EX;
reg next_pc_from_EX_eff_EX;
reg [3:0] rf_ri1_EX;
reg [3:0] rf_ri2_EX;
reg [11:0] pc_EX;
reg call_push_EX;
reg call_pop_EX;
reg bxxz_EX;
reg [1:0] branch_cond_EX;

// Unused, only for progation
reg [7:0] rf_rd1_EX;
reg [7:0] rf_rd2_EX;
reg sram_wrt_EX;
reg [9:0] imm10_EX;
reg operand_push_EX;
reg operand_pop_EX;
reg csr_bus_wrt_EX;
reg csr2rf_EX;
reg sram2rf_EX;
reg rf_wrt_EX;
reg [3:0] rf_wi_EX;

always @(posedge clk) begin
    // All signals except for PC are fresh
    imm2alu_EX <= imm2alu;
    alu_func_EX <= alu_func;
    rf_rd1_EX <= rf_rd1;
    rf_rd2_EX <= rf_rd2;
    sram_wrt_EX <= ~next_pc_from_EX_eff & sram_wrt;
    operand_pop_EX <= ~next_pc_from_EX_eff & operand_pop;
    operand_push_EX <= ~next_pc_from_EX_eff & operand_push;
    imm10_EX <= imm10;
    csr_bus_wrt_EX <= ~next_pc_from_EX_eff & csr_wrt;
    csr2rf_EX <= csr2rf;
    sram2rf_EX <= sram2rf;
    pc_EX <= pc_ID;
    call_push_EX <= ~next_pc_from_EX_eff & call_push;
    call_pop_EX <= ~next_pc_from_EX_eff & call_pop;
    next_pc_from_EX_eff_EX <= ~next_pc_from_EX_eff & next_pc_from_EX_eff_ID;
    rf_wrt_EX <= ~next_pc_from_EX_eff & rf_wrt;
    rf_wi_EX <= rf_wi;
    rf_ri1_EX <= rf_ri1;
    rf_ri2_EX <= rf_ri2;
    bxxz_EX <= ~next_pc_from_EX_eff & bxxz;
    branch_cond_EX <= branch_cond;
end

// Forwarding

wire [7:0] rf_rd1_EX_forwarded;
wire [7:0] rf_rd2_EX_forwarded;
wire bypass_rf_rd1_MEM2EX;
wire bypass_rf_rd1_WB2EX;
wire bypass_rf_rd2_MEM2EX;
wire bypass_rf_rd2_WB2EX;
wire [7:0] rf_rd1_MEM2EX;
wire [7:0] rf_rd1_WB2EX;
wire [7:0] rf_rd2_MEM2EX;
wire [7:0] rf_rd2_WB2EX;

assign bypass_rf_rd1_MEM2EX = ((rf_wi_MEM != 4'b0) & (rf_wi_MEM == rf_ri1_EX));
assign bypass_rf_rd1_WB2EX = ((rf_wi_WB != 4'b0) & (rf_wi_WB == rf_ri1_EX));
assign bypass_rf_rd2_MEM2EX = ((rf_wi_MEM != 4'b0) & (rf_wi_MEM == rf_ri2_EX));
assign bypass_rf_rd2_WB2EX = ((rf_wi_WB != 4'b0) & (rf_wi_WB == rf_ri2_EX));
assign rf_rd1_EX_forwarded = bypass_rf_rd1_MEM2EX ? rf_rd1_MEM2EX :
        (bypass_rf_rd1_WB2EX ? rf_rd1_WB2EX : rf_rd1_EX);
assign rf_rd2_EX_forwarded = bypass_rf_rd2_MEM2EX ? rf_rd2_MEM2EX :
        (bypass_rf_rd2_WB2EX ? rf_rd2_WB2EX : rf_rd2_EX);

wire [7:0] alu_in1, alu_in2;
assign alu_in1 = rf_rd1_EX_forwarded;
assign alu_in2 = imm2alu_EX ? imm10_EX[7:0] : rf_rd2_EX_forwarded;

alu alu (
    .a(alu_in1), 
    .b(alu_in2), 
    .z(alu_out), 
    .func(alu_func_EX)
);

wire [11:0] call_stack_out;

stack #( .WLEN(12), .DEPTH(8), .PTR_LEN(3) ) call_stack (
    .din(pc_EX), 
    .dout(call_stack_out), 
    .clk(clk), 
    .rst(rst), 
    .push(call_push_EX), 
    .pop(call_pop_EX)
);

wire next_pc_from_EX_eff;
wire [11:0] next_pc_from_EX;

npc_ex npc_ex (
    .prev_pc(pc_EX), 
    .rs_val(rf_rd1_EX_forwarded), 
    .imm10(imm10_EX), 
    .bxxz(bxxz_EX), 
    .branch_cond(branch_cond_EX), 
    .call_pop(call_pop_EX), 
    .call_stack_out(call_stack_out), 
    .branch_on_ex(next_pc_from_EX_eff_EX), 
    .next_pc_eff(next_pc_from_EX_eff), 
    .next_pc(next_pc_from_EX)
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
reg rf_wrt_MEM;
reg [3:0] rf_wi_MEM;

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
    rf_wrt_MEM <= rf_wrt_EX;
    rf_wi_MEM <= rf_wi_EX;
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

assign rf_rd1_MEM2EX = alu_out_MEM;
assign rf_rd2_MEM2EX = alu_out_MEM;

// ================ WB stage ================

// Pipelining latches, all of which are used
reg [7:0] alu_out_WB;
reg sram2rf_WB;
reg [7:0] sram_dat_WB;
reg operand_pop_WB;
reg [7:0] operand_stack_out_WB;
reg csr2rf_WB;
reg [7:0] csr_bus_dat_WB;
reg rf_wrt_WB;
reg [3:0] rf_wi_WB;

always @(posedge clk) begin
    // Fresh: CSRDat, SRAMDat, OSOut
    alu_out_WB <= alu_out_MEM;
    sram2rf_WB <= sram2rf_MEM;
    sram_dat_WB <= sram_dat;
    operand_pop_WB <= operand_pop_MEM;
    operand_stack_out_WB <= operand_stack_out;
    csr_bus_dat_WB <= csr_bus_dat;
    csr2rf_WB <= csr2rf_MEM;
    rf_wrt_WB <= rf_wrt_MEM;
    rf_wi_WB <= rf_wi_MEM;
end

wire [7:0] rf_wdat_WB;

assign rf_wdat_WB = sram2rf_WB ? sram_dat_WB : (operand_pop_WB ? operand_stack_out_WB : (csr2rf_WB ? csr_bus_dat_WB : alu_out_WB));

assign rf_rd1_WB2EX = rf_wdat_WB;
assign rf_rd2_WB2EX = rf_wdat_WB;

endmodule
