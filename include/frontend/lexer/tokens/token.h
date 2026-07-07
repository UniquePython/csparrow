#ifndef FRONTEND_LEXER_TOKENS_TOKEN_H_
#define FRONTEND_LEXER_TOKENS_TOKEN_H_

#include "span.h"
#include "frontend/lexer/tokens/tokenkind.h"

#include "tools/hashmap.h"

#include <string.h>
#include <stdbool.h>

typedef struct token_t
{
    TokenKind kind;
    char *value;
    Span span;
} Token;

Token newToken(TokenKind kind, char *value, Span span);
void freeToken(Token *token);

HASHMAP_DEFINE(char *, TokenKind, SCTM)
HASHMAP_DEFINE(char *, TokenKind, LATM)
HASHMAP_DEFINE(char *, TokenKind, KTTKM)
HASHMAP_DEFINE(TokenKind, char *, TDM)

extern SCTM *singleCharTokensMap;
extern LATM *lookAheadTokensMap;
extern KTTKM *keywordsToTokenKindMap;
extern TDM *tokenDisplayMap;

void initTokenTables(void);
void freeTokenTables(void);

#endif