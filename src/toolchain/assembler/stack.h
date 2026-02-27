#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdlib.h>

#define STACK_TYPE(T)\
    struct {\
        T *base; \
        uint_t top; \
        uint_t capacity;\
    }

#define STACK_INIT(T, S) {\
    S.capacity = 16;\
    S.top = 0;\
    if ((S.base = (typeof(S.base)) malloc(S.capacity * sizeof(T))) == NULL) {\
        fatal("Failed to create stack!");\
    }\
}

#define STACK_PUSH(S, E, T) {\
    if (S.top == S.capacity - 1) {\
        typeof(S.base) newBase = (typeof(S.base)) realloc(S.base, (S.capacity *= 2) * sizeof(T));\
        if (newBase == NULL) {\
            fatal("Failed to expand stack!");\
        } else {\
            S.base = newBase;\
        }\
    }\
    S.base[S.top++] = E;\
}

#define STACK_POP(S, E) {\
    if (S.top == 0) {\
        fatal("Stack underflow!");\
    }\
    E = S.base[--S.top];\
}

#define STACK_PEEK(S, E) {\
    if (S.top == 0) {\
        fatal("Stack underflow!");\
    }\
    E = S.base[S.top - 1];\
}

#define STACK_AS_ARRAY(S, A) {\
    A = S.base;\
}

#define STACK_SIZE(S, SZ) {\
    SZ = S.top;\
}

#define STACK_FREE(S) {\
    free(S.base);\
}

#endif // STACK_H_INCLUDED
