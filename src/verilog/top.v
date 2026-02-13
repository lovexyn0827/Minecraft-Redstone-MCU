// How about building a MACRO Controlling Unit? Here we go.

/** 
 * Let's us talk about the overall architecture:
 * 
 * - A small computer following the Harward's architecture (i.e. separate storage for data and instruction).
 * - A 8-bit datapath, 8-bit SRAM address, but 16 bit (or 12 bit) adressing space for program ROM.
 * - Unified address space of SRAM and display RAM, possibly with a DMA.
 * - 4096 bytes of ROM, with lower 3KiB for program and upper 1KiB for fixed constant.
 * - Hardware glyph library & stack with a visible pointer.
 * - RISC-like instruction set, fixed 16-bit, or else we have to struggle with microcode or FSMs. :shrug:
 * - RF contains 4x8 bit registers, or 8x8 bit as long as the instruction set permits.
 */

`define GPIN_CSR_NO     (0);
`define GPOUT_CSR_NO    (1);

module top (
    input   wire            clk, rst, 
    input   wire    [7:0]  gpin, 
    output  wire    [7:0]  gpout
);

wire [11:0] insn_addr;
wire [19:0] insn;

imem imem (
    .addr(insn_addr), 
    .insn(insn)
);

wire [7:0] sram_addr, sram_dat;
wire sram_wrt;

sram sram (
    .clk(clk), 
    .rst(rst), 
    .wrt(sram_wrt), 
    .addr(sram_addr), 
    .dat(sram_dat)
);

reg [7:0] csrs [1023:0];

assign gpout = csrs[1];

// For debugging purpose

wire [9:0] csr_bus_addr;
wire [7:0] csr_bus_dat;
wire csr_bus_wrt;

assign csr_bus_dat = csr_bus_wrt ? 8'bZZZZZZZZ : csrs[csr_bus_addr];

always @(posedge clk) begin
    if (csr_bus_wrt & csr_bus_addr[0]) begin
        csrs[csr_bus_addr] <= csr_bus_dat;
    end else begin
        csrs[0] <= gpin;
        csrs[2] <= gpout ^ 8'hFF;
    end
end

cpu cpu (
    .clk(clk), 
    .rst(rst), 
    .imem_addr(insn_addr), 
    .insn(insn), 
    .sram_addr(sram_addr), 
    .sram_wrt(sram_wrt), 
    .sram_dat(sram_dat), 
    .csr_bus_addr(csr_bus_addr), 
    .csr_bus_wrt(csr_bus_wrt), 
    .csr_bus_dat(csr_bus_dat)
);

endmodule
