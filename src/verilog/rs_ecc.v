/* verilator lint_off PINCONNECTEMPTY */

module rs_ecc # (
    parameter   N = 16,             // Message length
    parameter   K = 10,             // ECC length
    parameter   PP = 8'b00011101,   // Primitive
    parameter   GP = {
        8'd1, 8'd216, 8'd194, 8'd159, 8'd111, 8'd199, 8'd94, 8'd95, 8'd113, 8'd157, 8'd193
    },                              // Generator polynomial
    parameter   ADDR_LEN = 4, 

    parameter   CSR_RSDAT       = 10'h0D0,
    parameter   CSR_RSDATRDY    = 10'h0D1,
    parameter   CSR_RSECCRDY    = 10'h0D2,
    parameter   CSR_RSADDR      = 10'h0D3, 
    parameter   CSR_RSECC       = 10'h0D4, 
    parameter   CSR_RSRST       = 10'h0D5
) ( 
    input   wire                        clk, rst, 
    input   wire    [9:0]   csr_addr, 
    input   wire            csr_wrt, 
    input   wire    [7:0]   csr_dat_in,
    output  wire    [7:0]   csr_dat_out
);

wire [7:0] sdat_in;
wire sdat_in_rdy;
wire [ADDR_LEN - 1:0] ecc_addr;
wire rdy;
wire [7:0] ecc_out;
wire rs_rst;

csr_mosi # ( .ADDR(CSR_RSDAT), .WIDTH(8), .RST_VAL(8'b0) ) csr_rsdat (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_in), 
    .csr_dat_out(csr_dat_out), 
    .perh_out(sdat_in)
);

csr_mosi # ( .ADDR(CSR_RSDATRDY), .WIDTH(1), .RST_VAL(1'b0) ) csr_rsdatrdy (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_in[0]), 
    .csr_dat_out(csr_dat_out), 
    .perh_out(sdat_in_rdy)
);

csr_miso # ( .ADDR(CSR_RSECCRDY), .WIDTH(1), .RST_VAL(1'b0) ) csr_rseccrdy (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_dat_out(csr_dat_out), 
    .perh_wrt(1'b1), 
    .perh_in(rdy), 
    .perh_out()
);

csr_mosi # ( .ADDR(CSR_RSADDR), .WIDTH(ADDR_LEN), .RST_VAL(0) ) csr_rsaddr (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_in[ADDR_LEN - 1:0]), 
    .csr_dat_out(csr_dat_out), 
    .perh_out(ecc_addr)
);

csr_miso # ( .ADDR(CSR_RSECC), .WIDTH(8), .RST_VAL(8'b0) ) csr_rsecc (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_dat_out(csr_dat_out), 
    .perh_wrt(1'b1), 
    .perh_in(ecc_out), 
    .perh_out()
);

csr_mosi # ( .ADDR(CSR_RSRST), .WIDTH(1), .RST_VAL(1'b0) ) csr_rsrst (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_in[0]), 
    .csr_dat_out(csr_dat_out), 
    .perh_out(rs_rst)
);

rs_ecc_impl # (
    .N(N), .K(K), .PP(PP), .GP(GP), .ADDR_LEN(ADDR_LEN)
) impl (
    .clk(clk), 
    .rst(rst | rs_rst), 
    .sdat_in(sdat_in), 
    .sdat_in_rdy(sdat_in_rdy), 
    .ecc_addr(ecc_addr),
    .ecc_out(ecc_out), 
    .rdy(rdy)
);

endmodule
