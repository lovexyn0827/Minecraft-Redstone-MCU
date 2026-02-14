module cpu (
    input   wire            clk, rst, 

    output  wire    [11:0]  imem_addr, 
    input   wire    [19:0]  insn, 

    output  wire    [7:0]   sram_addr, 
    output  wire            sram_wrt,  
    inout   wire    [7:0]   sram_dat, 

    output  wire    [9:0]   csr_bus_addr, 
    output  wire            csr_bus_wrt, 
    inout   wire    [7:0]   csr_bus_dat
);

reg [11:0] pc;
wire [11:0] next_pc;

assign imem_addr = pc;

always @(posedge clk) begin
    pc <= rst ? 12'h010 : next_pc;
end

wire [3:0] alu_func;
wire operand_push, operand_pop;
wire call_push, call_pop;
wire [1:0] npc_mode; 
wire [11:0] imm12;
wire [3:0] rf_ri1, rf_ri2, rf_wi;
wire rf_wrt;
wire sram2rf;
wire imm2alu;
wire csr2rf;
wire jmp_with_reg;

wire [7:0] alu_out;
wire [7:0] rf_rd1, rf_rd2;

assign sram_addr = alu_out;
assign sram_dat = sram_wrt ? rf_rd2 : 8'bZZZZZZZZ;

assign csr_bus_dat = csr_bus_wrt ? rf_rd1 : 8'bZZZZZZZZ;
assign csr_bus_addr = imm12[9:0];

cu cu (
    .insn(insn), 
    .rs_val(rf_rd1), 
    .alu_func(alu_func), 
    .operand_push(operand_push), 
    .operand_pop(operand_pop), 
    .call_push(call_push), 
    .call_pop(call_pop), 
    .sram_wrt(sram_wrt), 
    .npc_mode(npc_mode), 
    .imm12_out(imm12), 
    .rf_ri1(rf_ri1), 
    .rf_ri2(rf_ri2), 
    .rf_wi(rf_wi), 
    .rf_wrt(rf_wrt), 
    .sram2rf(sram2rf), 
    .imm2alu(imm2alu), 
    .csr2rf(csr2rf),
    .csr_wrt(csr_bus_wrt), 
    .jmp_with_reg(jmp_with_reg)
);

wire [11:0] call_stack_out;

stack #( .WLEN(12) ) call_stack (
    .din(pc), 
    .dout(call_stack_out), 
    .clk(clk), 
    .rst(rst), 
    .push(call_push), 
    .pop(call_pop)
);

wire [11:0] npc_imm_target, npc_offset;
assign npc_imm_target = call_pop ? call_stack_out : imm12;
assign npc_offset = jmp_with_reg ? { { 4 { rf_rd1[7] } }, rf_rd1 } : imm12;

npc npc (
    .prev_pc(pc), 
    .imm_target(npc_imm_target), 
    .offset(npc_offset), 
    .mode(npc_mode), 
    .next_pc(next_pc)
);

wire [7:0] operand_stack_out;

stack #( .WLEN(8) ) operand_stack (
    .din(rf_rd1), 
    .dout(operand_stack_out), 
    .clk(clk), 
    .rst(rst), 
    .push(operand_push), 
    .pop(operand_pop)
);

wire [7:0] rf_wdat;
assign rf_wdat = sram2rf ? sram_dat : (operand_pop ? operand_stack_out : (csr2rf ? csr_bus_dat : alu_out));

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

wire [7:0] alu_in1, alu_in2;
assign alu_in1 = rf_rd1;
assign alu_in2 = imm2alu ? imm12[7:0] : rf_rd2;

alu alu (
    .a(alu_in1), 
    .b(alu_in2), 
    .z(alu_out), 
    .func(alu_func)
);

endmodule
