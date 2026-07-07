#ifndef TOOLS_CKDARITH_H_
#define TOOLS_CKDARITH_H_

#include "tools/fatal.h"

#define SPARROW_CHECKED_ADD(a, b, out)                         \
    do                                                         \
    {                                                          \
        if (__builtin_add_overflow((a), (b), (out)))           \
            SparrowFatal("integer overflow: %s + %s", #a, #b); \
    } while (0)

#define SPARROW_CHECKED_SUB(a, b, out)                         \
    do                                                         \
    {                                                          \
        if (__builtin_sub_overflow((a), (b), (out)))           \
            SparrowFatal("integer overflow: %s - %s", #a, #b); \
    } while (0)

#define SPARROW_CHECKED_MUL(a, b, out)                         \
    do                                                         \
    {                                                          \
        if (__builtin_mul_overflow((a), (b), (out)))           \
            SparrowFatal("integer overflow: %s * %s", #a, #b); \
    } while (0)

#endif