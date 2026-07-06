#ifndef SPAN_H_
#define SPAN_H_

#include <stdint.h>

typedef struct span_t
{
    uint64_t start;
    uint64_t end;
} Span;

#endif