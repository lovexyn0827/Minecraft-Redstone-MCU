module csr_mosi #(
    parameter   ADDR = 10'h00, 
    parameter   RST_VAL = 8'h00, 
    parameter   WIDTH = 4'd8
) (
    input   wire                    clk, rst, 
    input   wire    [9:0]           csr_addr, 
    input   wire                    csr_wrt, 
    input   wire    [WIDTH - 1:0]   csr_dat_in, 
    output  wire    [7:0]           csr_dat_out, 
    
    output  wire    [WIDTH - 1:0]   perh_out
);

reg [WIDTH - 1:0] backend;

assign perh_out = backend;
assign csr_dat_out = (csr_addr == ADDR) ? { { (8 - WIDTH) { 1'b0 } }, backend } : 8'bZZZZZZZZ;

always @(posedge clk) begin
    if (rst) begin
        backend <= RST_VAL;
    end else if (csr_wrt & (csr_addr == ADDR)) begin
        backend <= csr_dat_in[WIDTH - 1:0];
    end
end

endmodule
