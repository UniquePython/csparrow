#include "tools/strtools.h"

#include <string.h>

size_t SparrowStrLen(const char *s, size_t maxlen)
{
    size_t len = 0;

    while (len < maxlen && s[len] != '\0')
        len++;

    return len;
}

char *SparrowStrClone(const char *str, size_t n)
{
    size_t len = SparrowStrLen(str, n);

    char *copy = SparrowMalloc(len + 1);
    memcpy(copy, str, len);
    copy[len] = '\0';

    return copy;
}