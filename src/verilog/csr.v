module csr #(
    parameter   ADDR = 10'h00, 
    parameter   RST_VAL = 8'h00
) (
    input   wire            clk, rst, 
    input   wire    [9:0]   csr_addr, 
    input   wire            csr_wrt, 
    inout   wire    [7:0]   csr_dat, 
    
    input   wire            perh_wrt, 
    input   wire    [7:0]   perh_in, 
    output  wire    [7:0]   perh_out
);

reg [7:0] backend;

assign csr_dat = (~csr_wrt & (csr_addr == ADDR)) ? backend : 8'bZZZZZZZZ;
assign perh_out = backend;

always @(posedge clk) begin
    if (rst) begin
        backend <= RST_VAL;
    end else if (csr_wrt & (csr_addr == ADDR)) begin
        backend <= csr_dat;
    end else if (perh_wrt) begin
        backend <= perh_in;
    end
end

endmodule
