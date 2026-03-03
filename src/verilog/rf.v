module rf (
    input   wire    [3:0]   ri1, ri2, wi, 
    input   wire            clk, rst, wrt, 
    input   wire    [7:0]   wd, 
    output  wire    [7:0]   rd1, rd2
);

reg [7:0] regs[15:1];

assign rd1 = ri1 == 0 ? 8'b0 : regs[ri1];
assign rd2 = ri2 == 0 ? 8'b0 : regs[ri2];

always @(posedge clk) begin
    if (rst) begin
        for (integer i = 1; i < 16; i = i + 1) begin
            regs[i] <= 8'b0;
        end
    end else if (wrt & (wi != 4'b0)) begin
        regs[wi] <= wd;
    end
end

endmodule
