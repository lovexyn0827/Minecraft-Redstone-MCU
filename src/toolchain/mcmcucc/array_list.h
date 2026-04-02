#ifndef ARRAY_LIST_H_INCLUDED
#define ARRAY_LIST_H_INCLUDED

#include "common.h"

#define ARRAY_LIST_TYPE(T) struct {\
    T *base;\
    uint_t size;\
    uint_t capacity;\
}

#define ARRAY_LIST_INIT(T, L) {\
    (L).capacity = 16;\
    (L).size = 0;\
    if (((L).base = (typeof((L).base)) malloc((L).capacity * sizeof(T))) == NULL) {\
        fatal("Failed to create an array list!");\
    }\
}

#define ARRAY_LIST_APPEND(L, E, T) {\
    if ((L).size == (L).capacity - 1) {\
        typeof((L).base) newBase = (typeof((L).base)) realloc((L).base, ((L).capacity *= 2) * sizeof(T));\
        if (newBase == NULL) {\
            fatal("Failed to expand an array list!");\
        } else {\
            (L).base = newBase;\
        }\
    }\
    (L).base[(L).size++] = E;\
}

#define ARRAY_LIST_REMOVE_TAIL(L, E) {\
    if ((L).size == 0) {\
        fatal("List underflow!");\
    }\
    E = (L).base[--(L).size];\
}

#define ARRAY_LIST_GET_TAIL(L, E) {\
    if ((L).size == 0) {\
        fatal("Out of bound!");\
    }\
    E = (L).base[(L).size - 1];\
}

#define ARRAY_LIST_GET(L, E, I) {\
    uint _I = I;\
    if (_I < 0 || _I >= (L).size) {\
        fatal("Out of bound!");\
    }\
    E = (typeof(E)) (L).base[_I];\
}

#define ARRAY_LIST_SET(L, E, I) {\
    uint _I = I;\
    if (_I < 0 || _I >= (L).size) {\
        fatal("Out of bound!");\
    }\
    (L).base[_I] = (typeof((L).base[_I])) (E);\
}

#define ARRAY_LIST_TRAVERSE(L, T, V, I, OP) {\
    T V;\
    typeof(L) *_Lptr = &(L);\
    for (uint_t I = 0; I < _Lptr->size; I++) {\
        ARRAY_LIST_GET(L, V, I)\
        { OP }\
    }\
}

#define ARRAY_LIST_AS_ARRAY(L, A) {\
    A = (L).base;\
}

#define ARRAY_LIST_SIZE(L, SZ) {\
    SZ = (L).size;\
}

#define ARRAY_LIST_EMPTY_INLINE(L) ((L).size == 0)

#define ARRAY_LIST_FREE(L) {\
    free((L).base);\
}

#endif // ARRAY_LIST_H_INCLUDED
