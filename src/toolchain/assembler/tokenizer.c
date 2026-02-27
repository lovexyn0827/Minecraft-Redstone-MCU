#include "tokenizer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void init_token_list_builder(token_list_builder *builder, uint_t buffer_size) {
    STACK_INIT(str, builder->backend);
    builder->cur_token = (mutable_str) malloc(buffer_size);
    builder->cur_token_pos = 0;
}

void append_char_to_token(char_t ch, token_list_builder *builder) {
    builder->cur_token[builder->cur_token_pos++] = toupper(ch);
}

void next_token(token_list_builder *builder) {
    if (builder->cur_token_pos > 0) {
        mutable_str token = (mutable_str) malloc(builder->cur_token_pos + 1);
        builder->cur_token[builder->cur_token_pos] = '\0';
        strcpy(token, builder->cur_token);
        //printf("Token (%d): %s - %02x\n", builder->cur_token_pos, token, *token);
        STACK_PUSH(builder->backend, token, str)
        builder->cur_token_pos = 0;
    }
}

void build_token_list(token_list_builder *builder, read_head *list) {
    STACK_AS_ARRAY(builder->backend, list->ptr)
    uint_t token_cnt;
    STACK_SIZE(builder->backend, token_cnt)
    list->max = list->ptr + token_cnt;
}

char_t peek_char(FILE *fp) {
    char_t ch = fgetc(fp);
    ungetc(ch, fp);
    return ch;
}

void skip_blanks(FILE *fp) {
    char ch;
    do {
        ch = fgetc(fp);
    } while ((isblank(ch) || ch == '\r') && ch != EOF);
    if (ch != EOF) {
        ungetc(ch, fp);
    } else {
        fgetc(fp);
    }
}

void append_all_char_st(FILE *fp, token_list_builder *builder, bool (*st)(char_t ch)) {
    char_t ch;
    while (st(ch = fgetc(fp))) {
         append_char_to_token(ch, builder);
    }

    ungetc(ch, fp);
}

void skip_comments(FILE *fp) {
    char_t ch;
    while ((ch = fgetc(fp)) != '\n');
    ungetc(ch, fp);
}

bool is_char_allowed_in_hex(char_t ch) {
    return isdigit(ch) || (tolower(ch) >= 'a' && tolower(ch) <= 'f');
}

bool is_char_allowed_in_bin(char_t ch) {
    return ch == '0' || ch == '1';
}

bool is_char_allowed_in_oct(char_t ch) {
    return ch >= '0' && ch <= '7';
}

bool is_char_allowed_in_dec(char_t ch) {
    return isdigit(ch);
}

void parse_number_token(FILE *fp, token_list_builder *builder) {
    // Guarantees that the first character is in `[0-9]`
    if (peek_char(fp) == '0') {
        fgetc(fp);
        append_char_to_token('0', builder);
        switch (tolower(peek_char(fp))) {
        case 'x':
            // Hex
            fgetc(fp);
            append_char_to_token('x', builder);
            append_all_char_st(fp, builder, is_char_allowed_in_hex);
            break;
        case 'b':
            // Bin
            fgetc(fp);
            append_char_to_token('b', builder);
            append_all_char_st(fp, builder, is_char_allowed_in_bin);
            break;
        default:
            if (isdigit((uchar_t) peek_char(fp))) {
                // Oct
                append_all_char_st(fp, builder, is_char_allowed_in_dec);
            } else {
                // 0
                append_char_to_token('0', builder);
            }

            break;
        }
    } else {
        // Dec
        append_all_char_st(fp, builder, is_char_allowed_in_dec);
    }
}

bool is_char_allowed_in_identifiers(char_t ch) {
    return isalnum(ch) || ch == '_';
}

void parse_identifier_like_token(FILE *fp, token_list_builder *builder) {
    append_all_char_st(fp, builder, is_char_allowed_in_identifiers);
}

void parse_operator_token(FILE *fp, token_list_builder *builder) {
    append_char_to_token(fgetc(fp), builder);
    next_token(builder);
}

void tokenize_single(FILE *fp, token_list_builder *builder) {
    char_t first_ch = peek_char(fp);
    if (first_ch == EOF) {
        return;
    } else if (first_ch == '%') {
        skip_comments(fp);
    }else if (isdigit(first_ch)) {
        parse_number_token(fp, builder);
    } else if (isalpha(first_ch) || first_ch == '_') {
        parse_identifier_like_token(fp, builder);
    } else {
        parse_operator_token(fp, builder);
    }

    next_token(builder);
}

void tokenize(FILE *fp, token_list_builder *builder) {
    init_token_list_builder(builder, 4096);
    uint_t token_cnt = 0;
    while (!feof(fp)) {
        skip_blanks(fp);
        tokenize_single(fp, builder);
        skip_blanks(fp);
        token_cnt++;
    }

    debug("Captured %d tokens\n", token_cnt);
}
