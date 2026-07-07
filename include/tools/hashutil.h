#ifndef TOOLS_HASHUTIL_H_
#define TOOLS_HASHUTIL_H_

#include "frontend/tokens/tokenkind.h"

#include <stdint.h>
#include <stdbool.h>

uint64_t hashCStr(char **key);
bool eqCStr(char **a, char **b);

uint64_t hashTokenKind(TokenKind *key);
bool eqTokenKind(TokenKind *a, TokenKind *b);

#endif