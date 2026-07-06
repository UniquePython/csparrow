#ifndef TOOLS_ALLOCATOR_H_
#define TOOLS_ALLOCATOR_H_

#include <stdlib.h>

void *SparrowMalloc(size_t size);
void *SparrowCalloc(size_t nmemb, size_t size);
void *SparrowRealloc(void *ptr, size_t size);

#endif