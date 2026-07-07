#include "tools/hashutil.h"

#include <string.h>

static uint64_t hashCStr(char **key)
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

static bool eqCStr(char **a, char **b)
{
    return strcmp(*a, *b) == 0;
}

uint64_t hashTokenKind(const void *key)
{
    return (uint64_t)*(const TokenKind *)key;
}

bool eqTokenKind(const void *lhs, const void *rhs)
{
    return *(const TokenKind *)lhs == *(const TokenKind *)rhs;
}
