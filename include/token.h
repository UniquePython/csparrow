#ifndef TOKEN_H_
#define TOKEN_H_

#include "span.h"
#include "tokenkind.h"

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

uint64_t hashCStr(char **key);
bool eqCStr(char **a, char **b);

SCTM *singleCharTokensMap;
LATM *lookAheadTokensMap;
KTTKM *keywordsToTokenKindMap;
TDM *tokenDisplayMap;

void populateSCTM(void);
void populateLATM(void);
void populateKTTKM(void);
void populateTDM(void);

#endif