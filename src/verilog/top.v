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
    input   wire            clk, rst, 
    input   reg     [15:0]  gpin, 
    output  reg     [15:0]  gpout, 
    output  wire    [15:0]  display_mem
);



endmodule