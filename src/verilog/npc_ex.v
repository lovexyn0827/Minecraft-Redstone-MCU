module npc_ex (
    input   wire    [11:0]  prev_pc, 
    input   wire    [7:0]   rs_val, 
    input   wire    [9:0]   imm10, 
    input   wire            bxxz,  
    input   wire    [1:0]   branch_cond, 
    input   wire            call_pop, 
    input   wire    [11:0]  call_stack_out, 
    input   wire            branch_on_ex,
    output  wire            next_pc_eff, 
    output  wire    [11:0]  next_pc
);

wire should_branch;
assign should_branch = { rs_val == 8'b0, rs_val != 8'b0, ~rs_val[7], rs_val[7] } [branch_cond];

wire [11:0] branch_to;
assign branch_to = prev_pc + { { 2 { imm10[9] } }, imm10 };

wire [11:0] ljmp_to;
assign ljmp_to = prev_pc + { { 4 { rs_val[7] } }, rs_val };

assign next_pc_eff = branch_on_ex & ~(bxxz & ~should_branch);

assign next_pc = call_pop ? call_stack_out + 1 : (bxxz ? (should_branch ? branch_to : prev_pc) : ljmp_to);

endmodule
