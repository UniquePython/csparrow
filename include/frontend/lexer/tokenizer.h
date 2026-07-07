#ifndef FRONTEND_TOKENZER_H_
#define FRONTEND_TOKENIZER_H_

#include "frontend/lexer/tokens/token.h"

#include "tools/dynarray.h"

#include <stddef.h>

DYNARRAY_DEFINE(Token, TokenArray)

TokenArray *tokenize(const char *src, size_t srcLen);

#endif