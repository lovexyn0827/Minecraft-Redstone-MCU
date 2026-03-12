module csr_miso #(
    parameter   ADDR = 10'h00, 
    parameter   RST_VAL = 8'h00, 
    parameter   WIDTH = 4'd8
) (
    input   wire                    clk, rst, 
    input   wire    [9:0]           csr_addr, 
    output  wire    [7:0]           csr_dat_out, 
    
    input   wire                    perh_wrt, 
    input   wire    [WIDTH - 1:0]   perh_in, 
    output  wire    [WIDTH - 1:0]   perh_out
);

reg [WIDTH - 1:0] backend;

assign csr_dat_out = (csr_addr == ADDR) ? { { (8 - WIDTH) { 1'b0 } }, backend } : 8'bZZZZZZZZ;
assign perh_out = backend;

always @(posedge clk) begin
    if (rst) begin
        backend <= RST_VAL;
    end else if (perh_wrt) begin
        backend <= perh_in;
    end
end

endmodule
