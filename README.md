# Minecraft-Redstone-MCU

Resources for the MCU for the central museum of [my Minecraft 1.16 survival save](https://github.com/lovexyn0827/0827-Public-Notes/tree/master/1.16-Survival), including complete source code of the assembler, programs, and Verilog implementation.

Unless otherwise states, all source code in this repository is published under CC-0.

> Macro Controlling Units are also MCUs, isn't it.

## How to Get Started

## Hardware Architecture

 * A small computer following the Harward's architecture (i.e. separate storage for data and instruction).
 * A 8-bit datapath, 8-bit SRAM address, but 16 bit (or 12 bit) adressing space for program ROM.
 * Unified address space of SRAM and display RAM, possibly with a DMA.
 * 4096 bytes of ROM, with lower 3KiB for program and upper 1KiB for fixed constant.
 * Hardware glyph library & stack with a visible pointer.
 * RISC-like instruction set, fixed 16-bit, or else we have to struggle with microcode or FSMs. :shrug:
 * RF contains 4x8 bit registers, or 8x8 bit as long as the instruction set permits.

## Instruction Set and Assembly Language

WIP

## Program Usage

## Note
