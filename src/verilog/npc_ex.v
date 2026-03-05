module npc_ex (
    input   wire    [11:0]  prev_pc, 
    input   wire    [7:0]   rs_val, 
    input   wire    [11:0]  imm12, 
    input   wire            bxxz,  
    input   wire    [1:0]   branch_cond,
    input   wire            call_push,  
    input   wire            call_pop, 
    input   wire    [11:0]  call_stack_out, 
    input   wire            branch_on_ex,
    output  wire            next_pc_eff, 
    output  wire    [11:0]  next_pc
);

wire should_branch;
wire [3:0] branch_cond_impls;
assign branch_cond_impls = { rs_val[7], ~rs_val[7], rs_val != 8'b0, rs_val == 8'b0 };
assign should_branch = branch_cond_impls[branch_cond];

wire [11:0] branch_to;
assign branch_to = prev_pc + imm12;

wire [11:0] ljmp_to;
assign ljmp_to = imm12 + { { 4 { rs_val[7] } }, rs_val };

wire [11:0] invoke_target;
assign invoke_target = imm12 + { { 4 { rs_val[7] } }, rs_val };

assign next_pc_eff = branch_on_ex & ~(bxxz & ~should_branch);

assign next_pc = call_push ? invoke_target 
        : call_pop ? call_stack_out + 1 
        : bxxz ? (should_branch ? branch_to : prev_pc) : ljmp_to;

endmodule
