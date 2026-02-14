module imem (
    input   wire    [11:0]  addr, 
    output  reg     [19:0]  insn
);

// Simple test program:
// 
// ADDI     r1, r0, 10
// ISTORE   r1, 0x80(r1)
// ILOAD    r3, 0x8A(r0)
// ADD      r2, r0, r0
// ADD      r2, r3, r2
// ADDI     r3, r3, -1
// BNEZ     r3, -2
// OUTCSR   r2, 0x01
// ADDI     r4, r0, 0xCC
// PUSH     r4
// INCSR    r4, 0x02
// POP      r5
// INVOKE   
// LJMP     0x000
// ADDI     r1, r0, 0xF0
// RET

always @(addr) begin
    case (addr)
        12'h010: insn <= 20'h8010A;
        12'h011: insn <= 20'h31180;
        12'h012: insn <= 20'h2038A;
        12'h013: insn <= 20'h00020;
        12'h014: insn <= 20'h03220;
        12'h015: insn <= 20'h833FF;
        12'h016: insn <= 20'hE37FE;
        12'h017: insn <= 20'hF2C01;
        12'h018: insn <= 20'h804CC;
        12'h019: insn <= 20'h0400A;
        12'h01A: insn <= 20'hF4802;
        12'h01B: insn <= 20'h0005B;
        12'h01C: insn <= 20'hD001F;
        12'h01D: insn <= 20'h91020;
        12'h01E: insn <= 20'h801F0;
        12'h01F: insn <= 20'hF0000;
        default: insn <= 20'h00000;
    endcase
end

endmodule
