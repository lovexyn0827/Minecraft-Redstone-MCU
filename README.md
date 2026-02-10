# Minecraft-Redstone-MCU

Resources for the MCU for the central museum of [my Minecraft 1.16 survival save](https://github.com/lovexyn0827/0827-Public-Notes/tree/master/1.16-Survival), including complete source code of the assembler, programs, and Verilog implementation.

Unless otherwise states, all source code in this repository is published under CC-0.

> Macro Controlling Units are also MCUs, isn't it.

## How to Get Started

## Hardware Architecture

### Overview

 * A small computer following the Harvard's architecture (i.e. separate storage for data and instruction).
 * RISC-like instruction set, fixed 16-bit, or else we have to struggle with microcode or FSMs. :shrug:
 * Seven 8 bit registers, with read-only `r0` storing constant 0.
 * A 8-bit data-path, 8-bit SRAM address, but 12 bit addressing space for program ROM.
 * Unified address space of SRAM and display RAM, possibly with a DMA.
 * 4096 bytes of ROM, with lower 3 KB for program and upper 1 KB for fixed constant.
 * Memory-mapped IO, through CSRs.
 * Hardware glyph library & stack with a visible pointer.

### Memory Mapping

We have three separate address space for instructions, stack and data under manipulation.

Bytes can be transfered between two address spaces with some CSRs.

#### Instrument Memory

`````````
-------------------- 0xFFF
+  Sys. Routines   +
-------------------- 0xC00
+  User Constants  +
-------------------- 0x800
+                  +
+   User Programs  +
+                  +
-------------------- 0x000
`````````

Descriptions:

- User programs: User-defined program code. 
  - The program starts running at `0x010`. 
  - Lowest 4 words are for instruction vectors.
- User constants: User-defined constants. (e.g. Sine tables)
- System routines: Read-only region of code of system utilities. (e.g. Printing glyphs to the screen)

Note that this region is addressed by 16-bit words, that is, 2-byte pairs instead of individual bytes.

#### Stack

No specific structure, but a (theoretically unbounded) linear memory region with 8-bit words.

#### Data Memory

``````
-------------------- 0xFF
+   Display RAM    +
-------------------- 0xA0
+  Sys. Routines   +
-------------------- 0x80
+                  +
+   Ordinary Mem   +
+                  +
-------------------- 0x000
``````

### CSRs

The CSRs (Control State Registers) reside in the address range of `0x80 - 0x9F` in the data memory, and are the interface to all peripherals outside the CPU.

Unlike GPRs (i.e. `r0 - r7`), CSRs cannot be interacted directly with any instruction other than `ILOAD` and `ISTORE`, the ones you are likely to use when operating on the main memory.

The complete list of CSRs:

`````````
Addr.	Name  	Description
0x80	SP		Stack pointer
0x81	CSW		Control status sord
0x82-83	INTCFG	Interrupt config
0x84-85	GPICFG	General purpose input ports config
0x86-87 GPOCFG	General purpose output ports config
0x88-89 DPTR	R/W address or instruction memory
0x8A	IMLCK	Protection status of IM
0x8B	IMCTRL	Control status of IM	
0x8C-8D	IMDATA	Data buffer for the word to R/W into IM
0x8E	GPIBUF	General purpose input buffer
0x8F	GPOBUF	General purpose output buffer
0x90-97	GPIBIT	Per-bit access of GPI ports
0x98-9F	GPIBIT	Per-bit access of GPO ports
`````````

Unlisted addresses are not assigned to any hardware registers, thus, any operation on them will be no-op.

### Interrupts

There can be up to 4 interrupt sources in the MCU. No priority can be specified, but one may mask some interrupt sources or assign them to GPI pins to detect rising edges.

## ISA and Assembly Language

This MCU supports 29 instruments, all of which are 16-bits-long, and can be categorized into following types:

```````
         0123457689ABCDEF
R-Type: |op|rs|rt|rd|fnc| (3 + 3 + 3 + 3 + 4)
I-Type: |op|rs|rt| imm. | (3 + 3 + 3 + 7)
B-Type: |op|rs|f|  imm. | (3 + 3 + 2 + 8)
J-Type: |op|rs|  offset | (3 + 3 + A)
```````

### R-Type Instructions

```````
ADD  : 000 xxx xxx xxx 0000 - R[rd] <- R[rs] + R[rt]
SUB  : 000 xxx xxx xxx 0001 - R[rd] <- R[rs] - R[rt]
AND  : 000 xxx xxx xxx 0010 - R[rd] <- R[rs] & R[rt]
OR   : 000 xxx xxx xxx 0011 - R[rd] <- R[rs] | R[rt]
XOR  : 000 xxx xxx xxx 0101 - R[rd] <- R[rs] ^ R[rt]
CMP  : 000 xxx xxx xxx 0100 - R[rd] <- R[rs] == R[rt] ? 1 : 0
SHL  : 000 xxx xxx xxx 0110 - R[rd] <- R[rs] << R[rt]
SHR  : 000 xxx xxx xxx 0111 - R[rd] <- R[rs] >>> R[rt]
PUSH : 000 xxx 000 000 1000 - Stack[SP += 1] <- R[rs]
POP  : 000 000 000 xxx 1001 - R[rd] <- Stack[SP -= 1]
CLR  : 000 xxx xxx xxx 1010 - R[rd] <- R[rs] & ~(1 << R[rt]) 
SET  : 000 xxx xxx xxx 1011 - R[rd] <- R[rs] | (1 << R[rt])
CLRI : 000 xxx bit xxx 1100 - R[rd] <- R[rs] & ~(1 << bit) 
SETI : 000 xxx bit xxx 1101 - R[rd] <- R[rs] | (1 << bit) 
SHLI : 000 xxx shm xxx 1110 - R[rd] <- R[rs] << shm
SHRI : 000 xxx shm xxx 1111 - R[rd] <- R[rs] >>> shm
```````

### I-Type Instruments

``````
ADDI  : 001 xxx xxx xxxxxxx - R[rt] <- R[rs] + SignExt(imm)
ILOAD : 010 xxx xxx xxxxxxx - R[rt] <- Mem[R[rs] + ZeroExt(imm)] 
ISTORE: 011 xxx xxx xxxxxxx - Mem[R[rs] + ZeroExt(imm])] <- R[rt]
``````

### B-Type Instruments

``````
JE    : 100 xxx 00 xxxxxxxx - if (R[rs] == 8'b0)  PC <- PC + SignExt(imm) + 1
JNE   : 100 xxx 01 xxxxxxxx - if (R[rs] != 8'b0)  PC <- PC + SignExt(imm) + 1
JLT   : 100 xxx 10 xxxxxxxx - if (R[rs] >= 8'h80) PC <- PC + SignExt(imm) + 1
JGT   : 100 xxx 11 xxxxxxxx - if (R[rs] <= 8'hFF) PC <- PC + SignExt(imm) + 1
ICONST: 101 xxx 00 xxxxxxxx - R[rs] <- imm
STAJMP: 101 xxx 01 xxxxxxxx - PC <- (R[rs] << 4) + ZeroExt(imm) + 1
JLTU  : 101 xxx 10 xxxxxxxx - if (R[rs] >= 8'h80) PC <- PC + ZeroExt(imm) + 1
JGTU  : 101 xxx 11 xxxxxxxx - if (R[rs] <= 8'hFF) PC <- PC + ZeroExt(imm) + 1
``````

### J-Type Instruments

```````
JMP   : 110 xxx xxxxxxxxxx - PC <- PC + R[rs] + SignExt[imm] + 1
INVOKE: 111 xxx xxxxxxxxxx - PC <- R[rs] + SignExt[imm]; R[rs] <- PC_Prev
```````

## Program Usage

## Note
