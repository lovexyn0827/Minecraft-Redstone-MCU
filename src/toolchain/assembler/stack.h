#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#define STACK_TYPE(T)\
    struct {\
        T *base; \
        uint32_t top; \
        uint32_t capacity;\
    }

#define STACK_INIT(T, S)\
    S.capacity = 16;\
    S.top = 0;\
    if ((S.base = (T*) malloc(S.capacity * sizeof(T))) == NULL) {\
        error();\
    }

#define STACK_PUSH(S, E, T)\
    if (S.top == S.capacity - 1) {\
        T *newBase = (T*) realloc(S.base, S.capacity * sizeof(T));\
        if (newBase == NULL) {\
            error();\
        } else {\
            S.base = newBase;\
        }\
    }\
    S.base[S.top++] = E;\

#define STACK_POP(S, E)\
    if (S.top == 0) {\
        error();\
    }\
    E = S.base[--S.top];

#define STACK_PEEK(S, E)\
    if (S.top == 0) {\
        error();\
    }\
    E = S.base[S.top - 1];

#define STACK_FREE(S)\
    free(S.base);

#endif // STACK_H_INCLUDED
