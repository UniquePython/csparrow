#ifndef TOOLS_CKDARITH_H_
#define TOOLS_CKDARITH_H_

#include <stdio.h>
#include <stdlib.h>

#define SPARROW_CHECKED_ADD(a, b, out)                    \
    do                                                    \
    {                                                     \
        if (__builtin_add_overflow((a), (b), (out)))      \
        {                                                 \
            fprintf(stderr,                               \
                    "Fatal: integer overflow: %s + %s\n", \
                    #a, #b);                              \
            abort();                                      \
        }                                                 \
    } while (0)

#define SPARROW_CHECKED_SUB(a, b, out)                    \
    do                                                    \
    {                                                     \
        if (__builtin_sub_overflow((a), (b), (out)))      \
        {                                                 \
            fprintf(stderr,                               \
                    "Fatal: integer overflow: %s - %s\n", \
                    #a, #b);                              \
            abort();                                      \
        }                                                 \
    } while (0)

#define SPARROW_CHECKED_MUL(a, b, out)                    \
    do                                                    \
    {                                                     \
        if (__builtin_mul_overflow((a), (b), (out)))      \
        {                                                 \
            fprintf(stderr,                               \
                    "Fatal: integer overflow: %s * %s\n", \
                    #a, #b);                              \
            abort();                                      \
        }                                                 \
    } while (0)

#endif