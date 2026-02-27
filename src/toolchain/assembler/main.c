#include "tokenizer.h"
#include "parser.h"
#include "common.h"
#include "output.h"

#include <stdio.h>
#include <stdlib.h>

void init_infrastructures();
void parse_args(int_t argc, str *argv, bool *run_tests, bool *show_version, bool *show_bin,
                str *output, output_fmt *format, str *input);
int test_main();
void show_version_info();
int main0(str input, str output, output_fmt format, bool run_tests, bool show_version, bool show_bin);

uint_t error_cnt = 0;
uint_t warning_cnt = 0;

int main(int argc, str *argv) {
    init_infrastructures();
    if (argc == 1) {
        // Test cases
        return test_main();
    } else {
        bool run_tests, show_version, show_bin;
        output_fmt format;
        str output, input;
        parse_args(argc, argv, &run_tests, &show_version, &show_bin, &output, &format, &input);
        return main0(input, output, format, run_tests, show_version, show_bin);
    }
}

void init_infrastructures() {
    init_parser();
}

void parse_args(int_t argc, str *argv, bool *run_tests, bool *show_version, bool *show_bin,
                str *output, output_fmt *format, str *input) {
    *run_tests = false;
    *show_version = false;
    *output = NULL;
    *format = FMT_BINARY;
    *input = NULL;
    *show_bin = false;
    argv++;

    // asm [-t] [-v] [-o file] [-f format] -i input
    // -t: Run tests
    // -v: Version
    // -o: Assemble and output
    // -f: Output format (default: bin)
    //      b: Binary IM image of 32-bit words
    //      v: Verilog module of IM
    //      l: Logisim ROM image file
    // -i: Input
    // -s: Show binary
    while (*argv != NULL) {
        str arg = *(argv++);
        switch (arg[1]) {
        case 't':
            *run_tests = true;
            break;
        case 'v':
            *show_version = true;
            break;
        case 'o':
            *output = *(argv++);
            break;
        case 'f':
            if (*(argv) != NULL) *format = (output_fmt) (**(argv++));
            break;
        case 'i':
            *input = *(argv++);
            break;
        case 's':
            *show_bin = true;
            break;
        default:
            error("Unrecognized switch: %s\n", arg);
        }
    }
}

void free_tokens(read_head *ptr) {
    str *start = ptr->ptr;
    for (str *cur = ptr->ptr; cur < ptr->max; cur++) {
        free((void*) (*cur));
    }

    free(start);
}

int main0(str input, str output, output_fmt format, bool run_tests, bool show_version, bool show_bin) {
    error_cnt = warning_cnt = 0;
    if (run_tests) {
        test_main();
    }

    if (show_version) {
        show_version_info();
    }

    if (input == NULL) {
        puts("No input file given!");
        return 0;
    }

    // Tokenize
    FILE *fp = fopen(input, "r");
    if (fp == NULL) {
        fatal("Failed to open %s!\n", input);
    }

    token_list_builder builder;
    tokenize(fp, &builder);
    fclose(fp);

    // Parse
    read_head tokens;
    build_token_list(&builder, &tokens);
    insn_t *insns = (insn_t*) malloc(IM_CAPACITY * sizeof(insn_t));
    for (int i = 0; i < IM_CAPACITY; i++) insns[i] = 0;
    uint_t sram_usage, im_usage;
    parse(&tokens, insns, &sram_usage, &im_usage);
    build_token_list(&builder, &tokens);
    free_tokens(&tokens);
    if (show_bin) {
        for (int i = PROG_ENTRY; i < 32; i++) {
            printf("%03x: %05x\n", i, insns[i]);
        }
    }

    // Output
    if (output != NULL && error_cnt == 0) {
        create_output(output, format, insns);
    }

    free(insns);

    // Report
    info("Assembly completed!\n");
    info("%u / %u bytes of SRAM used\n", sram_usage, SRAM_CAPACITY);
    info("%u / %u words of IM used\n", im_usage, IM_CAPACITY);
    info("%u Errors, %d Warnings\n", error_cnt, warning_cnt);

    return 0;
}

void test_single(str file_in) {
    main0(file_in, "D:/ASM_TMP", FMT_BINARY, false, false, true);
}

int test_main() {
    test_single("D:/RS_ECC.ASM");
    test_single("D:/INSNS.AMS");
    test_single("D:/MCMCU.ASM");
    test_single("D:/FIB.ASM");
    return 0;
}

void show_version_info() {
    info("Assembler for MC-MCU");
}
