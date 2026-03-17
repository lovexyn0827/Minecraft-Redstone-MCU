#include "tokenizer.h"

#include "map.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char_t fpeek(FILE *fp) {
    char_t first_ch = fgetc(fp);
    ungetc(first_ch, fp);
    return first_ch;
}

void read_while(FILE *fp, mutable_str buf, bool (*cond)(char_t)) {
    while (cond(fpeek(fp))) {
        *(buf++) = fgetc(fp);
    }

    *buf = '\0';
}

bool is_whitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

bool is_dec_digit(char_t ch) {
    return isdigit(ch);
}

bool is_bin_digit(char_t ch) {
    return ch == '0' || ch == '1';
}

bool is_hex_digit(char_t ch) {
    return isdigit(ch) || (tolower(ch) >= 'a' && tolower(ch) <= 'f');
}

token_type_t read_dec_number(FILE *fp, mutable_str buf) {
    read_while(fp, buf, is_dec_digit);
    return TOKEN_CONST_DEC;
}

token_type_t read_bin_number(FILE *fp, mutable_str buf) {
    read_while(fp, buf, is_bin_digit);
    return TOKEN_CONST_BIN;
}

token_type_t read_hex_number(FILE *fp, mutable_str buf) {
    read_while(fp, buf, is_hex_digit);
    return TOKEN_CONST_HEX;
}

token_type_t read_number(FILE *fp, mutable_str buf) {
    // The first character is 0 - 9
    if (fpeek(fp) == '0') {
        buf[0] = fgetc(fp);
        switch (fpeek(fp)) {
        case 'x':
        case 'X':
            buf[1] = fgetc(fp);
            return read_hex_number(fp, buf + 2);
        case 'b':
        case 'B':
            buf[1] = fgetc(fp);
            return read_bin_number(fp, buf + 2);
        default:
            buf[1] = '\0';
            return TOKEN_CONST_DEC;
        }
    } else {
        return read_dec_number(fp, buf);
    }
}

bool is_valid_in_identifier(char_t ch) {
    return isalpha(ch) || ch == '_' || isdigit(ch);
}

HASH_MAP_TYPE(str, token_type_t) KEYWORD_TOKEN_TYPES;

void init_keyword_token_type_map() {
    HASH_MAP_INIT(str, token_type_t, KEYWORD_TOKEN_TYPES, 64, hash_str, TOKEN_IDENTIFIER)
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "break", TOKEN_KW_BREAK, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "case", TOKEN_KW_CASE, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "const", TOKEN_KW_CONST, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "continue", TOKEN_KW_CONTINUE, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "default", TOKEN_KW_DEFAULT, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "do", TOKEN_KW_DO, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "else", TOKEN_KW_ELSE, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "for", TOKEN_KW_FOR, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "goto", TOKEN_KW_GOTO, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "if", TOKEN_KW_IF, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "inline", TOKEN_KW_INLINE, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "int8_t", TOKEN_KW_INT8_T, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "uint8_t", TOKEN_KW_UINT8_T, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "likely", TOKEN_KW_LIKELY, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "register", TOKEN_KW_REGISTER, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "return", TOKEN_KW_RETURN, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "sizeof", TOKEN_KW_SIZEOF, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "switch", TOKEN_KW_SWITCH, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "unlikely", TOKEN_KW_UNLIKELY, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "void", TOKEN_KW_VOID, str, token_type_t, str_equal);
    HASH_MAP_PUT(KEYWORD_TOKEN_TYPES, "while", TOKEN_KW_WHILE, str, token_type_t, str_equal);
}

token_type_t read_identifier_like(FILE *fp, mutable_str buf) {
    // The first character is in [A-Za-z_]
    read_while(fp, buf, is_valid_in_identifier);
    token_type_t token_type;
    HASH_MAP_GET(KEYWORD_TOKEN_TYPES, buf, token_type, str, token_type, str_equal)
    return token_type;
}

token_type_t read_char_constant(FILE *fp, mutable_str buf) {
    // Starting with '
    buf[0] = fgetc(fp);
    if (fpeek(fp) == '\\') {
        buf[1] = fgetc(fp);
        buf[2] = fgetc(fp);
        buf[3] = fgetc(fp);
        buf[4] = '\0';
    } else {
        buf[1] = fgetc(fp);
        buf[2] = fgetc(fp);
        buf[3] = '\0';
    }

    return TOKEN_CONST_CHAR;
}

HASH_MAP_TYPE(str, token_type_t) PUNCTUATOR_TOKEN_TYPES;

void init_punctuator_token_type_map() {
    HASH_MAP_INIT(str, token_type_t, PUNCTUATOR_TOKEN_TYPES, 256, hash_str, TOKEN_ERROR)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "[", TOKEN_PUNCT_L_SP, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "]", TOKEN_PUNCT_R_SP, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "(", TOKEN_PUNCT_L_P, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ")", TOKEN_PUNCT_R_P, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "{", TOKEN_PUNCT_L_CP, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "}", TOKEN_PUNCT_R_CP, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "++", TOKEN_PUNCT_INC, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "--", TOKEN_PUNCT_DEC, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "&", TOKEN_PUNCT_AND, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "*", TOKEN_PUNCT_STAR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "+", TOKEN_PUNCT_PLUS, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "-", TOKEN_PUNCT_MINUS, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "~", TOKEN_PUNCT_NEG, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "!", TOKEN_PUNCT_NOT, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "/", TOKEN_PUNCT_SLASH, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "%", TOKEN_PUNCT_PERCENT, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<<", TOKEN_PUNCT_SHL, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ">>", TOKEN_PUNCT_SHR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<", TOKEN_PUNCT_LT, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ">", TOKEN_PUNCT_GT, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<=", TOKEN_PUNCT_LE, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ">=", TOKEN_PUNCT_GE, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "==", TOKEN_PUNCT_EQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "!=", TOKEN_PUNCT_NE, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "^", TOKEN_PUNCT_XOR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "|", TOKEN_PUNCT_OR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "&&", TOKEN_PUNCT_LAND, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "||", TOKEN_PUNCT_LOR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "?", TOKEN_PUNCT_QUESTION, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ":", TOKEN_PUNCT_COLON, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ";", TOKEN_PUNCT_SEMICOLON, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "=", TOKEN_PUNCT_ASSIGN, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "*=", TOKEN_PUNCT_MULEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "/=", TOKEN_PUNCT_DIVEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "%=", TOKEN_PUNCT_MODEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "+=", TOKEN_PUNCT_PLUSEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "-=", TOKEN_PUNCT_MINUSEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<<=", TOKEN_PUNCT_SHLEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ">>=", TOKEN_PUNCT_SHREQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "&=", TOKEN_PUNCT_ANDEQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "^=", TOKEN_PUNCT_XOR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "|=", TOKEN_PUNCT_OREQ, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<|", TOKEN_PUNCT_SET, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "<&", TOKEN_PUNCT_CLR, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, ",", TOKEN_PUNCT_COMMA, str, token_type_t, str_equal)
    HASH_MAP_PUT(PUNCTUATOR_TOKEN_TYPES, "//", TOKEN_PUNCT_COMMENT, str, token_type_t, str_equal)
}

token_type_t read_punctuator(FILE *fp, mutable_str buf) {
    token_type_t token_type = TOKEN_ERROR;
    token_type_t prev_token_type;
    uint_t i = 0;
    do {
        buf[i++] = fgetc(fp);
        buf[i] = '\0';
        prev_token_type = token_type;
        HASH_MAP_GET(PUNCTUATOR_TOKEN_TYPES, buf, token_type, str, token_type_t, str_equal);
    } while (token_type != TOKEN_ERROR);
    ungetc(buf[--i], fp);
    buf[i] = '\0';
    return prev_token_type;
}

// token := keyword | identifier | number | punctuator

token_type_t read_one_token(FILE *fp, mutable_str buf) {
    char_t first_ch = fpeek(fp);
    if (isdigit(first_ch)) {
        return read_number(fp, buf);
    } else if (isalpha(first_ch) || first_ch == '_') {
        return read_identifier_like(fp, buf);
    } else if (first_ch == '\'') {
        return read_char_constant(fp, buf);
    } else {
        return read_punctuator(fp, buf);
    }
}

uint_t skip_whitespaces(FILE *fp, uint_t *column) {
    char_t ch;
    uint_t new_line_count = 0;
    while (!feof(fp) && is_whitespace(ch = fgetc(fp))) {
        if (ch == '\n') {
            new_line_count++;
            *column = 0;
        } else {
            (*column)++;
        }
    }

    if (!feof(fp)) {
        column--;
        ungetc(ch, fp);
    }

    return new_line_count;
}

void skip_to_next_line(FILE *fp) {
    while (!feof(fp) && fgetc(fp) != '\n');
}

bool tokenizer_static_initialized = false;

void init_tokenizer() {
    init_keyword_token_type_map();
    init_punctuator_token_type_map();
    tokenizer_static_initialized = true;
}

token_t* create_token(token_type_t token_type, str char_buf, uint_t line_num, uint_t column) {
    token_t * new_token = (token_t*) malloc(sizeof(token_t));
    new_token->type = token_type;
    new_token->line_num = line_num;
    new_token->column_pos = column;
    mutable_str token_str = (mutable_str) malloc(sizeof(char_t) * strlen(char_buf));
    strcpy(token_str, char_buf);
    new_token->token = token_str;
    return new_token;
}

void tokenize(FILE *fp, token_lst_t *token_lst) {
    if (!tokenizer_static_initialized) init_tokenizer();
    uint_t line_num = 1;
    uint_t column = 0;
    while (!feof(fp)) {
        skip_whitespaces(fp, &column);
        char_t char_buf[1024];
        uint_t prev_pos = ftell(fp);
        token_type_t token_type = read_one_token(fp, char_buf);
        uint_t token_len = ftell(fp) - prev_pos;
        column += token_len;
        if (strlen(char_buf) == 0) continue;
        if (token_type == TOKEN_PUNCT_COMMENT) {
            skip_to_next_line(fp);
            line_num++;
            column = 0;
        } else if (token_type == TOKEN_ERROR) {
            error("Unrecognized token: ", token_type);
        } else {
            token_t *new_token = create_token(token_type, char_buf, line_num, column);
            ARRAY_LIST_APPEND(*token_lst, new_token, const token_t*);
        }
    }

    token_t *eof_token = create_token(TOKEN_EOF, "", line_num, column);
    ARRAY_LIST_APPEND(*token_lst, eof_token, const token_t*);
}
