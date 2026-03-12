/* verilator lint_off PINCONNECTEMPTY */

module gpio (
    input   wire            clk, rst, 
    input   wire    [9:0]   csr_addr, 
    input   wire            csr_wrt, 
    input   wire    [7:0]   csr_dat_in,
    output  wire    [7:0]   csr_dat_out, 

    input   wire    [7:0]   gpin, 
    output  wire    [7:0]   gpout
);

csr_miso # ( .ADDR(10'h000) ) gpin_dat (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_dat_out(csr_dat_out), 
    .perh_wrt(1'b1), 
    .perh_in(gpin), 
    .perh_out()
);

csr_mosi # ( .ADDR(10'h001) ) gpout_dat (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_in), 
    .csr_dat_out(csr_dat_out), 
    .perh_out(gpout)
);

endmodule
