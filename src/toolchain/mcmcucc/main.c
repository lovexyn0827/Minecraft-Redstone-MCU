#include "tokenizer.h"

#include <stdio.h>

uint_t error_cnt;
uint_t warning_cnt;

void debug_print_tokens(token_lst_t *token_lst) {
    uint_t size;
    ARRAY_LIST_SIZE(*token_lst, size);
    token_t const **tokens;
    ARRAY_LIST_AS_ARRAY(*token_lst, tokens);
    uint_t prev_line_num = 1;
    for (uint_t i = 0; i < size; i++) {
        const token_t *cur_token = tokens[i];
        for (uint_t j = 0; j < cur_token->line_num - prev_line_num; j++) debug("\n");
        prev_line_num = cur_token->line_num;
        debug("%s ", cur_token->token);
    }
}

int main(int argc, char ** argv) {
    FILE *fp = fopen("D:/rs_ecc.c", "r");
    if (fp == NULL) {
        fatal("Failed to open %s!\n", "D:/rs_ecc.c");
    }

    token_lst_t token_lst;
    ARRAY_LIST_INIT(token_t, token_lst);
    tokenize(fp, &token_lst);
    debug_print_tokens(&token_lst);
    fclose(fp);
}
