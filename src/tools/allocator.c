#include "tools/allocator.h"
#include "tools/fatal.h"

#include <stdlib.h>

void *SparrowMalloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        SparrowFatal("malloc(%zu) failed", size);

    return ptr;
}

void *SparrowCalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);

    if (ptr == NULL)
        SparrowFatal("calloc(%zu, %zu) failed", nmemb, size);

    return ptr;
}

void *SparrowRealloc(void *ptr, size_t size)
{
    void *newPtr = realloc(ptr, size);

    if (newPtr == NULL)
        SparrowFatal("realloc(%p, %zu) failed", ptr, size);

    return newPtr;
}