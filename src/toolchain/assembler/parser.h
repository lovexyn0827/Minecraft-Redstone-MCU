#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "map.h"
#include "common.h"

#define MAX_REGNUM (15)

// Token list
typedef struct {
    str *ptr;
    str * max;
} read_head;

bool inbounds(read_head *ptr);
void assert_inbounds(read_head *ptr);
str peek_current(read_head *ptr);
str read_current(read_head *ptr);
void skip_current(read_head *ptr);

typedef uint32_t insn_t;

typedef struct {
    HASH_MAP_TYPE(str, uint_t) constants;
    HASH_MAP_TYPE(str, uint_t) pointers;
    HASH_MAP_TYPE(str, uint_t) labels;
    uint_t next_var_ptr;
    uint_t next_insn_ptr;
    insn_t *insns;
    bool *sram_memmap;
    bool *im_memmap;
} parse_ctx;

int_t get_constant(parse_ctx *ctx, str name);
uint_t get_pointer(parse_ctx *ctx, str name);
uint_t get_label(parse_ctx *ctx, str name);

void init_parser();

void parse(read_head *ptr, uint_t *insn_buf, uint_t *sram_usage, uint_t *im_usage);

#endif // PARSER_H_INCLUDED
