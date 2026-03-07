#include "parser.h"

#include "output.h"

void create_binary(FILE *fp, insn_t *insns) {
    fwrite(insns, sizeof(insn_t), IM_CAPACITY, fp);
}

void create_verilog(FILE *fp, insn_t *insns) {
    // module imem (
    //     input   wire    [11:0]  addr,
    //     output  reg     [19:0]  insn
    // );
    // always @(addr) begin
    //     case (addr)
    //         12'h010: insn <= 20'h8010A;
    //         // ...
    //         default: insn <= 20'h00000;
    //     endcase
    // end
    // endmodule
    fprintf(fp, "module imem (\n");
    fprintf(fp, "    input wire [11:0] addr,\n");
    fprintf(fp, "    output  reg [19:0] insn\n");
    fprintf(fp, ");\n");
    fprintf(fp, "\n");
    fprintf(fp, "always @(addr) begin\n");
    fprintf(fp, "    case (addr)\n");
    for (uint_t i = 0; i < IM_CAPACITY; i++) {
        insn_t insn = insns[i];
        if (insn != INSN_NOP) {
            fprintf(fp, "        12'h%03x: insn <= 20'h%05x;\n", i, insn);
        }
    }

    fprintf(fp, "        default: insn <= 20'h00000;\n");
    fprintf(fp, "    endcase\n");
    fprintf(fp, "end\n");
    fprintf(fp, "\n");
    fprintf(fp, "endmodule\n\n");
}

void create_logisim(FILE *fp, insn_t *insns) {
    fprintf(fp, "v3.0 hex words addressed");
    for (int i = 0; i < IM_CAPACITY; i += 8) {
        fprintf(fp, "\n%03x: ", i);
        for (int j = 0; j < 8; j++) {
            fprintf(fp, "%05x ", insns[i + j]);
        }
    }
}

void create_output(str dest, output_fmt fmt, insn_t *insns) {
    FILE *fp = fopen(dest, "wb");
    if (fp == NULL) {
        fatal("Failed to open %s!\n", dest);
    }

    switch (fmt) {
    case FMT_BINARY:
        create_binary(fp, insns);
        break;
    case FMT_VERILOG:
        create_verilog(fp, insns);
        break;
    case FMT_LOGISIM:
        create_logisim(fp, insns);
        break;
    }

    fclose(fp);
}
