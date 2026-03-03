module stack #(
    parameter WLEN = 8,
    parameter DEPTH = 64, 
    parameter PTR_LEN = 6
) (
    input   wire    [WLEN - 1:0]    din, 
    input   wire                    push, pop, clk, rst, 
    output  wire    [WLEN - 1:0]    dout
);

reg [PTR_LEN - 1:0] pointer;

reg [WLEN - 1:0] regs[DEPTH - 1:0];

assign dout = regs[pointer];

always @(posedge clk) begin
    if (rst) begin
        pointer <= 0;
        for (integer i = 0; i < DEPTH; i = i + 1) begin
            regs[i] <= 0;
        end
    end else if (push) begin
        regs[pointer + 1'b1] <= din;
        pointer <= pointer + 1'b1;
    end else if (pop) begin
        pointer <= pointer - 1'b1;
    end
end

endmodule
