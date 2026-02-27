#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <stdio.h>
#include <ctype.h>

#include "stack.h"
#include "parser.h"
#include "common.h"

typedef struct {
    STACK_TYPE(str) backend;
    mutable_str cur_token;
    uint_t cur_token_pos;
} token_list_builder;

void build_token_list(token_list_builder *builder, read_head *list);

void tokenize(FILE *fp, token_list_builder *builder);

#endif // TOKENIZER_H_INCLUDED
