module sram (
    input   wire    [7:0]   addr, 
    input   wire            clk, rst, wrt, 
    inout   wire    [7:0]   dat
);

reg [7:0] regs[255:0];

assign dat = wrt ? 8'bZZZZZZZZ : regs[addr];

always @(posedge clk) begin
    if (rst) begin
        for (integer i = 0; i < 256; i = i + 1) begin
            regs[i] <= 8'b0;
        end
    end else if (wrt) begin
        regs[addr] <= dat;
    end
end

endmodule
