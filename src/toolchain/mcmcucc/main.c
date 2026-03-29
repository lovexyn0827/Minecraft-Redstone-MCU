#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>

#include "context.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "opt_passes.h"

uint_t error_cnt;
uint_t warning_cnt;

bool verbose;

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

    putchar('\n');
}

void print_error_cnt_and_exit() {
    printf("%d Errors, %d Warnings\n", error_cnt, warning_cnt);
    exit(error_cnt > 0 ? -1 : 0);
}

int compile(str source, str output) {
    FILE *fp = fopen(source, "r");
    if (fp == NULL) {
        perror("");
        fatal("Failed to open %s!\n", source);
    }

    token_lst_t token_lst;
    ARRAY_LIST_INIT(token_t, token_lst);
    tokenize(fp, &token_lst);
    if (verbose) debug_print_tokens(&token_lst);
    fclose(fp);
    if (error_cnt > 0) {
        print_error_cnt_and_exit();
    }

    context_t ctx;
    init_compilation_context(&ctx, &token_lst);
    parse(&ctx);
    // ...
    if (error_cnt > 0) {
        print_error_cnt_and_exit();
    }

    check_semantics(&ctx);
    if (error_cnt > 0) {
        print_error_cnt_and_exit();
    }

    optimize_ast(&ctx);
    if (error_cnt > 0) {
        print_error_cnt_and_exit();
    }

    print_error_cnt_and_exit();
    return 0;
}

int main(int argc, char ** argv) {
    verbose = true;
    return compile("/home/lovexyn0827/Minecraft-Redstone-MCU/src/test/toolchain/mcmcucc/mcmcucc_test.c", "out.asm");
}
