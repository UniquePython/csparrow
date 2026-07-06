#include "tools/allocator.h"

#include <stdio.h>
#include <stdlib.h>

void *SparrowMalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
    {
        fprintf(stderr, "Fatal: malloc(%zu) failed\n", size);
        abort();
    }

    return ptr;
}

void *SparrowCalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);

    if (ptr == NULL)
    {
        fprintf(stderr, "Fatal: calloc(%zu, %zu) failed\n", nmemb, size);
        abort();
    }

    return ptr;
}

void *SparrowRealloc(void *ptr, size_t size)
{
    void *newPtr = realloc(ptr, size);

    if (newPtr == NULL)
    {
        fprintf(stderr, "Fatal: realloc(%p, %zu) failed\n", ptr, size);
        abort();
    }

    return newPtr;
}