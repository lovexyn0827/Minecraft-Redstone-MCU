/* verilator lint_off PINCONNECTEMPTY */

module gpio (
    input   wire            clk, rst, 
    input   wire    [9:0]   csr_addr, 
    input   wire            csr_wrt, 
    inout   wire    [7:0]   csr_dat, 

    input   wire    [7:0]   gpin, 
    output  wire    [7:0]   gpout
);

csr # ( .ADDR(10'h000) ) gpin_dat (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat(csr_dat), 
    .perh_wrt(1'b1), 
    .perh_in(gpin), 
    .perh_out()
);

csr # ( .ADDR(10'h001) ) gpout_dat (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat(csr_dat), 
    .perh_wrt(1'b0),
    .perh_in(),  
    .perh_out(gpout)
);

endmodule
