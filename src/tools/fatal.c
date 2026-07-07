#include "tools/fatal.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void SparrowFatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Fatal: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);

    abort();
}