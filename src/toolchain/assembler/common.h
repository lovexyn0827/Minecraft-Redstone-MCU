#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>

#define IM_CAPACITY (2048)
#define SRAM_CAPACITY (256)

#define UNDEFINED_CONSTANT (0x80000000)

#define PROG_ENTRY (0x010)

typedef char char_t;
typedef uint8_t uchar_t;
typedef const char_t *str;
typedef char_t *mutable_str;
typedef uint32_t uint_t;
typedef int32_t int_t;

void fatal(str fmt, ...);
void error(str fmt, ...);
void warn(str fmt, ...);
void info(str fmt, ...);
void debug(str fmt, ...);

uint_t hash_str(str in);
bool str_equal(str s1, str s2);

#endif // COMMON_H_INCLUDED
