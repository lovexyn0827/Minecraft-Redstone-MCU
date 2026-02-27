module kb_drv (
    input   wire            clk, rst, 
    input   wire    [9:0]   csr_addr, 
    input   wire            csr_wrt, 
    inout   wire    [7:0]   csr_dat, 

    input   wire    [63:0]  keys
);

// Key -----> FIFO --> KBDAT
//       \--> PIC

endmodule
