# Minecraft-Redstone-MCU

Resources for the MCU for the central museum of [my Minecraft 1.16 survival save](https://github.com/lovexyn0827/0827-Public-Notes/tree/master/1.16-Survival), including complete source code of the assembler, programs, and Verilog implementation.

Unless otherwise states, all source code in this repository is published under CC-0.

> Macro Controlling Units are also MCUs, isn't it?

## How to Get Started

## Hardware Architecture

### Overview

 * A small computer following the Harvard's architecture (i.e. separate storage for data and instruction).
 * RISC-like instruction set, fixed 19-bit, or else we have to struggle with microcode or FSMs. :shrug:
 * 16 8-bit registers, with read-only `r0` storing constant 0.
 * A 8-bit data-path, 8-bit SRAM address, but 12 bit addressing space for program ROM.
 * Unified address space of SRAM and display RAM, possibly with a DMA.
 * 4096 bytes of ROM, with lower 3 KB for program and upper 1 KB for fixed constant.
 * Memory-mapped IO, through CSRs.
 * Hardware glyph library & stack with a visible pointer.

### Memory Mapping

We have three separate address space for instructions, stack, CSRs and data under manipulation.

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

Note that this lower 2 KB and higher 1 KB for instructions of this region is addressed by 19-bit words, and the middle 1 KB is addressed by 16-bit words, neither of which is byte-addressable.

#### Stack

No specific structure, but a (theoretically unbounded) linear memory region with 8-bit words.

#### IO Ports (CSRs)

The CSRs (Control State Registers) reside in the address range of `0x00 - 0xFF` of the IO port space.

Unlike GPRs (i.e. `r0 - r7`), CSRs cannot be interacted directly with any instruction other than `IOCSR`. It is up to the hardware to determine whether to read from / write to CSRs by their address.

The complete list of CSRs:

`````````
Addr.	Name  	I/O	Description
0x00-01	PC		I	Program counter
0x80	SP		I	Stack pointer
0x81	CSW		I	Control status sord
0x82-83	INTCFG	O	Interrupt config
0x84-85	GPICFG	O	General purpose input ports config
0x86-87 GPOCFG	O	General purpose output ports config
0x88-89 DPTR	O	R/W address or instruction memory
0x8A	IMLCK	O	Protection status of IM
0x8B	IMCTRL	O	Control status of IM	
0x8C-8D	IMDATA	I	Data buffer for the word to R/W into IM
0x8E	GPIBUF	I	General purpose input buffer
0x8F	GPOBUF	O	General purpose output buffer
0x90-97	GPIBIT	I	Per-bit access of GPI ports
0x98-9F	GPIBIT	O	Per-bit access of GPO ports
`````````

Unlisted addresses are not assigned to any hardware backend, thus, any operation on them will be no-op.

#### Data Memory

``````
-------------------- 0xFF
+    Display RAM   +
-------------------- 0xA0
+                  +
+   Ordinary Mem   +
+                  +
-------------------- 0x00
``````

### Interrupts

There can be up to 2 interrupt sources in the MCU. No priority can be specified, but one may mask some interrupt sources or assign them to GPI pins to detect various edges.

This feature may be dropped if it takes to much to switch between contexts. (`INTPC`, etc.?)

## ISA and Assembly Language

This MCU supports 29 instruments, all of which are 19-bits-long, and can be categorized into following types:

```````
         0123457689ABCDEFGHI
R-Type: |op| rs| rt| rd|fnc| (3 + 4 + 4 + 4 + 4)
I-Type: |op| rs| rt|  imm. | (3 + 4 + 4 + 8)
B-Type: |op| rs|fn|  imm.  | (3 + 4 + 2 + 10)
J-Type: |op| rs|  offset   | (3 + 4 + 12)
```````

> 19-bits instruction may seem weird, but it is an possibly an optimal compromise after trying 16-bits designs but resulting in insufficient number of GPRs and complex jumping instructions.

### R-Type Instructions

```````
ADD  : 000 xxxx xxxx xxxx 0000 - R[rd] <- R[rs] + R[rt]
SUB  : 000 xxxx xxxx xxxx 0001 - R[rd] <- R[rs] - R[rt]
AND  : 000 xxxx xxxx xxxx 0010 - R[rd] <- R[rs] & R[rt]
OR   : 000 xxxx xxxx xxxx 0011 - R[rd] <- R[rs] | R[rt]
XOR  : 000 xxxx xxxx xxxx 0101 - R[rd] <- R[rs] ^ R[rt]
CMP  : 000 xxxx xxxx xxxx 0100 - R[rd] <- R[rs] == R[rt] ? 1 : 0
SHL  : 000 xxxx xxxx xxxx 0110 - R[rd] <- R[rs] << R[rt]
SHR  : 000 xxxx xxxx xxxx 0111 - R[rd] <- R[rs] >>> R[rt]
PUSH : 000 xxxx 0000 0000 1000 - Stack[SP += 1] <- R[rs]
POP  : 000 0000 0000 xxxx 1001 - R[rd] <- Stack[SP -= 1]
CLR  : 000 xxxx xxxx xxxx 1010 - R[rd] <- R[rs] & ~(1 << R[rt]) 
SET  : 000 xxxx xxxx xxxx 1011 - R[rd] <- R[rs] | (1 << R[rt])
CLRI : 000 xxxx 0bit xxxx 1100 - R[rd] <- R[rs] & ~(1 << bit) 
SETI : 000 xxxx 0bit xxxx 1101 - R[rd] <- R[rs] | (1 << bit) 
SHLI : 000 xxxx 0shm xxxx 1110 - R[rd] <- R[rs] << shm
SHRI : 000 xxxx 0shm xxx0 1111 - R[rd] <- R[rs] >>> shm
```````

### I-Type Instruments

``````
ADDI  : 001 xxxx xxxx xxxxxxxx - R[rt] <- R[rs] + imm
ILOAD : 010 xxxx xxxx xxxxxxxx - R[rt] <- Mem[R[rs] + imm] 
ISTORE: 011 xxxx xxxx xxxxxxxx - Mem[R[rs] + imm)] <- R[rt]
``````

### B-Type Instruments

``````
BE    : 100 xxxx 00 xxxxxxxxx - if (R[rs] == 8'b0)  PC <- PC + SignExt(imm) + 1
BNE   : 100 xxxx 01 xxxxxxxxx - if (R[rs] != 8'b0)  PC <- PC + SignExt(imm) + 1
BLT   : 100 xxxx 10 xxxxxxxxx - if (R[rs] >= 8'h80) PC <- PC + SignExt(imm) + 1
BGT   : 100 xxxx 11 xxxxxxxxx - if (R[rs] <= 8'hFF) PC <- PC + SignExt(imm) + 1
ICONST: 101 xxxx 00 xxxxxxxxx - R[rs] <- imm
SLTI  : 101 xxxx 01 xxxxxxxxx - R[rs] <- R[rs] < imm ? 1 : 0
INCSR : 101 xxxx 10 xxxxxxxxx - CSR[imm] <- R[rs]
OUTCSR: 101 xxxx 11 xxxxxxxxx - r[rs] <- CSR[imm]
``````

### J-Type Instruments

```````
JMP   : 110 xxxx xxxxxxxxxxxx - PC <- PC + R[rs] + imm + 1
INVOKE: 111 xxxx xxxxxxxxxxxx - PC <- R[rs] + imm; { R[15], R[14] } <- { 4'b0, PC_Prev }
```````

## Program Usage

## Note
