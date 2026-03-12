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

module top (
    input   wire           clk, rst, 
    input   wire    [7:0]  gpin, 
    output  wire    [7:0]  gpout
);

wire [11:0] insn_addr;
wire [19:0] insn;

imem imem (
    .addr(insn_addr), 
    .insn(insn)
);

wire [7:0] sram_addr, sram_dat_mosi, sram_dat_miso;
wire sram_wrt;

sram sram (
    .clk(clk), 
    .rst(rst), 
    .wrt(sram_wrt), 
    .addr(sram_addr), 
    .dat_in(sram_dat_mosi), 
    .dat_out(sram_dat_miso)
);

wire [9:0] csr_addr;
wire csr_wrt;
wire [7:0] csr_dat_mosi, csr_dat_miso;

gpio gpio (
    .clk(clk), 
    .rst(rst), 
    .csr_addr(csr_addr), 
    .csr_wrt(csr_wrt), 
    .csr_dat_in(csr_dat_mosi), 
    .csr_dat_out(csr_dat_miso), 
    .gpin(gpin), 
    .gpout(gpout)
);

cpu cpu (
    .clk(clk), 
    .rst(rst), 
    .imem_addr(insn_addr), 
    .insn(insn), 
    .sram_addr(sram_addr), 
    .sram_wrt(sram_wrt), 
    .sram_dat_in(sram_dat_miso), 
    .sram_dat_out(sram_dat_mosi), 
    .csr_bus_addr(csr_addr), 
    .csr_bus_wrt(csr_wrt), 
    .csr_bus_dat_in(csr_dat_miso), 
    .csr_bus_dat_out(csr_dat_mosi),
    .csr_bus_intr(1'b0)
);

endmodule
