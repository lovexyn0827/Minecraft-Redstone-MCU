`define NOP (20'h00000)

module cpu (
    input   wire            clk, rst, 

    output  wire    [11:0]  imem_addr, 
    input   wire    [19:0]  insn, 

    output  wire    [7:0]   sram_addr, 
    output  wire            sram_wrt,  
    input   wire    [7:0]   sram_dat_in, 
    output  wire    [7:0]   sram_dat_out, 

    output  wire    [9:0]   csr_bus_addr , 
    output  wire            csr_bus_wrt,
    input   wire            csr_bus_intr,  
    input   wire    [7:0]   csr_bus_dat_in, 
    output  wire    [7:0]   csr_bus_dat_out
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
    insn_ID <= (next_pc_from_ID_eff | next_pc_from_EX_eff) ? `NOP : insn;;
    intr_ID <= jump_intr;
    pc_ID <= pc;
end

wire insert_bubble;
assign insert_bubble = next_pc_from_EX_eff 
        | (sram2rf_EX & ((rf_ri1_ID == rf_wi_EX) | (rf_ri2_ID == rf_wi_EX)))
        | (csr2rf_EX & ((rf_ri1_ID == rf_wi_EX) | (rf_ri2_ID == rf_wi_EX)))
        | (operand_pop_EX & ((rf_ri1_ID == rf_wi_EX) | (rf_ri2_ID == rf_wi_EX)))
        | (sram2rf_MEM & ((rf_ri1_ID == rf_wi_MEM) | (rf_ri2_ID == rf_wi_MEM)))
        | (csr2rf_MEM & ((rf_ri1_ID == rf_wi_MEM) | (rf_ri2_ID == rf_wi_MEM)))
        | (operand_pop_MEM & ((rf_ri1_ID == rf_wi_MEM) | (rf_ri2_ID == rf_wi_MEM)));

wire call_push_ID, call_pop_ID;
wire bxxz_ID;
wire [1:0] branch_cond_ID;

// We determine when to generate branch targets at ID - and targets themselves at ID or EX
// So that we are not referencing R[rs] or the call stack at this moment
wire next_pc_from_ID_eff;
wire [11:0] next_pc_from_ID;
wire next_pc_from_EX_eff_ID;

wire jump_insn_at_ID;
wire [11:0] next_pc_from_insn_ID;

assign next_pc_from_ID_eff = jump_insn_at_ID | insert_bubble;
assign next_pc_from_ID = insert_bubble ? pc_ID : next_pc_from_insn_ID;

npc_id npc_id (
    .prev_pc(pc), 
    .insn_19_to_16(insn_ID[19:16]), 
    .insn_11_to_0(insn_ID[11:0]), 
    .intr(intr), 
    .call_push(call_push_ID), 
    .call_pop(call_pop_ID), 
    .iret(iret), 
    .jump_at_id(jump_insn_at_ID), 
    .jump_at_ex(next_pc_from_EX_eff_ID), 
    .bxxz(bxxz_ID), 
    .branch_cond(branch_cond_ID), 
    .next_pc(next_pc_from_insn_ID)
);

wire [3:0] alu_func_ID;
wire operand_push_ID, operand_pop_ID;
wire [11:0] imm12_ID;
wire [3:0] rf_ri1_ID, rf_ri2_ID, rf_wi_ID;
wire rf_wrt_ID;
wire sram2rf_ID;
wire sram_wrt_ID;
wire imm2alu_ID;
wire csr2rf_ID;
wire csr_wrt_ID;

wire [7:0] rf_rd1_ID, rf_rd2_ID;

cu cu (
    .insn(insn_ID), 
    .intr(intr_ID), 
    .alu_func(alu_func_ID), 
    .sram_wrt(sram_wrt_ID), 
    .imm12_out(imm12_ID), 
    .operand_push(operand_push_ID), 
    .operand_pop(operand_pop_ID),
    .rf_ri1(rf_ri1_ID), 
    .rf_ri2(rf_ri2_ID), 
    .rf_wi(rf_wi_ID), 
    .rf_wrt(rf_wrt_ID), 
    .sram2rf(sram2rf_ID), 
    .imm2alu(imm2alu_ID), 
    .csr2rf(csr2rf_ID),
    .csr_wrt(csr_wrt_ID)
);

rf rf (
    .clk(clk), 
    .rst(rst), 
    .wrt(rf_wrt_WB), 
    .ri1(rf_ri1_ID), 
    .ri2(rf_ri2_ID), 
    .wi(rf_wi_WB), 
    .wd(rf_wdat_WB), 
    .rd1(rf_rd1_ID), 
    .rd2(rf_rd2_ID)
);

// Forwarding

wire [7:0] rf_rd1_ID_forwarded;
wire [7:0] rf_rd2_ID_forwarded;
wire bypass_rf_rd1_EX2ID;
wire bypass_rf_rd1_MEM2ID;
wire bypass_rf_rd1_WB2ID;
wire bypass_rf_rd2_EX2ID;
wire bypass_rf_rd2_MEM2ID;
wire bypass_rf_rd2_WB2ID;
wire [7:0] rf_rd1_EX2ID;
wire [7:0] rf_rd1_MEM2ID;
wire [7:0] rf_rd1_WB2ID;
wire [7:0] rf_rd2_EX2ID;
wire [7:0] rf_rd2_MEM2ID;
wire [7:0] rf_rd2_WB2ID;

wire should_not_forward_MEM2ID;
assign should_not_forward_MEM2ID = sram2rf_MEM | csr2rf_MEM | operand_pop_MEM;

assign bypass_rf_rd1_EX2ID = rf_wrt_EX & ((rf_wi_EX != 4'b0) & (rf_wi_EX == rf_ri1_ID));
assign bypass_rf_rd1_MEM2ID = ~should_not_forward_MEM2ID & rf_wrt_MEM & ((rf_wi_MEM != 4'b0) & (rf_wi_MEM == rf_ri1_ID));
assign bypass_rf_rd1_WB2ID = rf_wrt_WB & ((rf_wi_WB != 4'b0) & (rf_wi_WB == rf_ri1_ID));
assign bypass_rf_rd2_EX2ID = rf_wrt_EX & ((rf_wi_EX != 4'b0) & (rf_wi_EX == rf_ri2_ID));
assign bypass_rf_rd2_MEM2ID = ~should_not_forward_MEM2ID & rf_wrt_MEM & ((rf_wi_MEM != 4'b0) & (rf_wi_MEM == rf_ri2_ID));
assign bypass_rf_rd2_WB2ID = rf_wrt_WB & ((rf_wi_WB != 4'b0) & (rf_wi_WB == rf_ri2_ID));

assign rf_rd1_ID_forwarded = bypass_rf_rd1_EX2ID ? rf_rd1_EX2ID
        : bypass_rf_rd1_MEM2ID ? rf_rd1_MEM2ID 
        : bypass_rf_rd1_WB2ID ? rf_rd1_WB2ID : rf_rd1_ID;
assign rf_rd2_ID_forwarded = bypass_rf_rd2_EX2ID ? rf_rd2_EX2ID
        : bypass_rf_rd2_MEM2ID ? rf_rd2_MEM2ID 
        : bypass_rf_rd2_WB2ID ? rf_rd2_WB2ID : rf_rd2_ID;

// ================ EX stage ================

// Pipelining latches
reg imm2alu_EX;
reg [3:0] alu_func_EX;
reg next_pc_from_EX_eff_EX;
reg [11:0] pc_EX;
reg call_push_EX;
reg call_pop_EX;
reg bxxz_EX;
reg [1:0] branch_cond_EX;
reg [11:0] imm12_EX;

// Unused, only for progation
reg [7:0] rf_rd1_EX;
reg [7:0] rf_rd2_EX;
reg sram_wrt_EX;
reg operand_push_EX;
reg operand_pop_EX;
reg csr_bus_wrt_EX;
reg csr2rf_EX;
reg sram2rf_EX;
reg rf_wrt_EX;
reg [3:0] rf_wi_EX;

always @(posedge clk) begin
    // All signals except for PC are fresh
    imm2alu_EX <= imm2alu_ID;
    alu_func_EX <= alu_func_ID;
    rf_rd1_EX <= rf_rd1_ID_forwarded;
    rf_rd2_EX <= rf_rd2_ID_forwarded;
    sram_wrt_EX <= ~insert_bubble & sram_wrt_ID;
    operand_pop_EX <= ~insert_bubble & operand_pop_ID;
    operand_push_EX <= ~insert_bubble & operand_push_ID;
    imm12_EX <= imm12_ID;
    csr_bus_wrt_EX <= ~insert_bubble & csr_wrt_ID;
    csr2rf_EX <= csr2rf_ID;
    sram2rf_EX <= sram2rf_ID;
    pc_EX <= pc_ID;
    call_push_EX <= ~insert_bubble & call_push_ID;
    call_pop_EX <= ~insert_bubble & call_pop_ID;
    next_pc_from_EX_eff_EX <= ~insert_bubble & next_pc_from_EX_eff_ID;
    rf_wrt_EX <= ~insert_bubble & rf_wrt_ID;
    rf_wi_EX <= rf_wi_ID;
    bxxz_EX <= ~insert_bubble & bxxz_ID;
    branch_cond_EX <= branch_cond_ID;
end

wire [7:0] alu_in1, alu_in2;
assign alu_in1 = rf_rd1_EX;
assign alu_in2 = imm2alu_EX ? imm12_EX[7:0] : rf_rd2_EX;

wire [7:0] alu_out_EX;

alu alu (
    .a(alu_in1), 
    .b(alu_in2), 
    .z(alu_out_EX), 
    .func(alu_func_EX)
);

wire [11:0] call_stack_out_EX;

stack #( .WLEN(12), .DEPTH(8), .PTR_LEN(3) ) call_stack (
    .din(pc_EX), 
    .dout(call_stack_out_EX), 
    .clk(clk), 
    .rst(rst), 
    .push(call_push_EX), 
    .pop(call_pop_EX)
);

wire next_pc_from_EX_eff;
wire [11:0] next_pc_from_EX;

npc_ex npc_ex (
    .prev_pc(pc_EX), 
    .rs_val(rf_rd1_EX), 
    .imm12(imm12_EX), 
    .bxxz(bxxz_EX), 
    .branch_cond(branch_cond_EX), 
    .call_push(call_push_EX), 
    .call_pop(call_pop_EX), 
    .call_stack_out(call_stack_out_EX), 
    .branch_on_ex(next_pc_from_EX_eff_EX), 
    .next_pc_eff(next_pc_from_EX_eff), 
    .next_pc(next_pc_from_EX)
);

assign rf_rd1_EX2ID = alu_out_EX;
assign rf_rd2_EX2ID = alu_out_EX;

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
    alu_out_MEM <= alu_out_EX;
    rf_rd1_MEM <= rf_rd1_EX;
    rf_rd2_MEM <= rf_rd2_EX;
    sram2rf_MEM <= sram2rf_EX;
    sram_wrt_MEM <= sram_wrt_EX;
    operand_pop_MEM <= operand_pop_EX;
    operand_push_MEM <= operand_push_EX;
    imm10_MEM <= imm12_EX[9:0];
    csr_bus_wrt_MEM <= csr_bus_wrt_EX;
    csr2rf_MEM <= csr2rf_EX;
    rf_wrt_MEM <= rf_wrt_EX;
    rf_wi_MEM <= rf_wi_EX;
end

wire [7:0] operand_stack_out_MEM;

stack #( .WLEN(8), .DEPTH(32), .PTR_LEN(5) ) operand_stack (
    .din(rf_rd1_MEM), 
    .dout(operand_stack_out_MEM), 
    .clk(clk), 
    .rst(rst), 
    .push(operand_push_MEM), 
    .pop(operand_pop_MEM)
);

assign sram_addr = alu_out_MEM;
assign sram_dat_out = rf_rd2_MEM;
assign sram_wrt = sram_wrt_MEM;

assign csr_bus_dat_out = rf_rd1_MEM;
assign csr_bus_addr = imm10_MEM;
assign csr_bus_wrt = csr_bus_wrt_MEM;

assign rf_rd1_MEM2ID = alu_out_MEM;
assign rf_rd2_MEM2ID = alu_out_MEM;

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
    sram_dat_WB <= sram_dat_in;
    operand_pop_WB <= operand_pop_MEM;
    operand_stack_out_WB <= operand_stack_out_MEM;
    csr_bus_dat_WB <= csr_bus_dat_in;
    csr2rf_WB <= csr2rf_MEM;
    rf_wrt_WB <= rf_wrt_MEM;
    rf_wi_WB <= rf_wi_MEM;
end

wire [7:0] rf_wdat_WB;

assign rf_wdat_WB = sram2rf_WB ? sram_dat_WB : (operand_pop_WB ? operand_stack_out_WB : (csr2rf_WB ? csr_bus_dat_WB : alu_out_WB));

assign rf_rd1_WB2ID = rf_wdat_WB;
assign rf_rd2_WB2ID = rf_wdat_WB;

endmodule
