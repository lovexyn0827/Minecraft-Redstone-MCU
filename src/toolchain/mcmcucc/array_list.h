#ifndef ARRAY_LIST_H_INCLUDED
#define ARRAY_LIST_H_INCLUDED

#include "common.h"

#define ARRAY_LIST_TYPE(T) struct {\
    T *base;\
    uint_t size;\
    uint_t capacity;\
}

#endif // ARRAY_LIST_H_INCLUDED
