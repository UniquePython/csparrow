#ifndef FRONTEND_TOKENZER_H_
#define FRONTEND_TOKENIZER_H_

#include "frontend/lexer/tokens/token.h"

#include "tools/dynarray.h"

#include <stdbool.h>
#include <stddef.h>

DYNARRAY_DEFINE(Token, TokenArray)

bool IsIdentifierStart(char c);
bool IsIdentifier(char c);

TokenArray tokenize(char *src, size_t srcLen);

#endif