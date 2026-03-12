module sram (
    input   wire    [7:0]   addr, 
    input   wire            clk, rst, wrt, 
    input   wire    [7:0]   dat_in,
    output  wire    [7:0]   dat_out
);

reg [7:0] regs[255:0];

assign dat_out = regs[addr];

always @(posedge clk) begin
    if (rst) begin
        for (integer i = 0; i < 256; i = i + 1) begin
            regs[i] <= 8'b0;
        end
    end else if (wrt) begin
        regs[addr] <= dat_in;
    end
end

endmodule
