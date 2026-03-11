#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint_t error_cnt, warning_cnt;

void fatal(str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(1);
}

void error(str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    error_cnt++;
}

void warn(str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    warning_cnt++;
}

void info(str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void debug(str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

uint_t hash_str(str in) {
    if (in == NULL) return 0;
    uint_t hash = 0;
    for (int i = 0; i < strlen(in); i++) {
        hash = 31 * hash + in[i];
    }

    return hash;
}

bool str_equal(str s1, str s2) {
    return strcmp(s1, s2) == 0;
}
