#ifndef OUTPUT_H_INCLUDED
#define OUTPUT_H_INCLUDED

#include <stdio.h>

#include "instructions.h"
#include "common.h"
#include "parser.h"

typedef enum {
    FMT_BINARY = 'b',
    FMT_VERILOG = 'v',
    FMT_LOGISIM = 'l'
} output_fmt;

void create_output(str dest, output_fmt fmt, insn_t *insns);

#endif // OUTPUT_H_INCLUDED
