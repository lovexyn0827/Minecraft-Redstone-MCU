module rf (
    input   wire    [3:0]   ri1, ri2, wi, 
    input   wire            clk, rst, wrt, 
    input   wire    [7:0]   wd, 
    output  wire    [7:0]   rd1, rd2
);

reg [7:0] regs[15:1];

assign rd1 = ri1 == 0 ? 8'b0 : regs[ri1];
assign rd2 = regs[ri2];

always @(posedge clk) begin
    if (rst) begin
        regs[1] <= 8'b0;
        regs[2] <= 8'b0;
        regs[3] <= 8'b0;
        regs[4] <= 8'b0;
        regs[5] <= 8'b0;
        regs[6] <= 8'b0;
        regs[7] <= 8'b0;
        regs[8] <= 8'b0;
        regs[9] <= 8'b0;
        regs[10] <= 8'b0;
        regs[11] <= 8'b0;
        regs[12] <= 8'b0;
        regs[13] <= 8'b0;
        regs[14] <= 8'b0;
        regs[15] <= 8'b0;
    end else if (wrt & (wi != 4'b0)) begin
        regs[wi] <= wd;
    end
end

endmodule
