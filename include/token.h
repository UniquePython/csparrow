#ifndef TOKEN_H_
#define TOKEN_H_

#include "span.h"
#include "tokenkind.h"

typedef struct token_t
{
    TokenKind kind;
    char *value;
    Span span;
} Token;

Token newToken(TokenKind kind, char *value, Span span);
void freeToken(Token *token);

#endif