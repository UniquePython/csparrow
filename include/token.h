#ifndef TOKEN_H_
#define TOKEN_H_

#include "span.h"
#include "tokenkind.h"

#include <stdbool.h>

typedef struct token_t
{
    TokenKind kind;
    char *value;
    Span span;
    bool isInit;
} Token;

Token newToken(TokenKind kind, char *value, Span span);
void freeToken(Token *token);

#endif