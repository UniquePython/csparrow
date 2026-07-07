#include "tools/hashutil.h"

#include <string.h>

uint64_t hashCStr(char **key)
{
    uint64_t h = 1469598103934665603ULL; // FNV-1a
    const char *s = *key;
    while (*s)
    {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

bool eqCStr(char **a, char **b)
{
    return strcmp(*a, *b) == 0;
}

uint64_t hashTokenKind(TokenKind *key)
{
    return (uint64_t)(*key);
}

bool eqTokenKind(TokenKind *a, TokenKind *b)
{
    return *a == *b;
}
