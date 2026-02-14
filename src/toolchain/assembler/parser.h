#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "map.h"

typedef uint8_t *str;

// Token list
typedef struct {
    const str *ptr;
    const str const *max;
} read_head;

void error();

bool inbounds(read_head *ptr);

void assert_inbounds(read_head *ptr);

const str peek_current(read_head *ptr);

void skip_current(read_head *ptr);

typedef struct {
    MAP_TYPE(str, uint32_t) constants;
    MAP_TYPE(str, uint32_t) pointers;
    MAP_TYPE(str, uint32_t) labels;
    uint32_t next_var_ptr;
    uint32_t next_insn_ptr;
    uint32_t *insns;
} parse_ctx;

#endif // PARSER_H_INCLUDED
