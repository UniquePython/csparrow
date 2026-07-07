#include "frontend/lexer/tokens/token.h"

#include "tools/allocator.h"
#include "tools/strtools.h"
#include "tools/ckdarith.h"
#include "tools/hashutil.h"

#include <stdlib.h>
#include <stdint.h>

Token newToken(TokenKind kind, char *value, Span span)
{
    return (Token){
        .kind = kind,
        .value = value,
        .span = span,
    };
}

void freeToken(Token *token)
{
    free(token->value);
    token->value = NULL;
}

SCTM *singleCharTokensMap = NULL;
LATM *lookAheadTokensMap = NULL;
KTTKM *keywordsToTokenKindMap = NULL;
TDM *tokenDisplayMap = NULL;

void populateSCTM(void)
{
    singleCharTokensMap = SCTM_new(hashCStr, eqCStr, NULL, NULL);

    SCTM_put(singleCharTokensMap, "+", TK_PLUS);
    SCTM_put(singleCharTokensMap, "-", TK_MINUS);
    SCTM_put(singleCharTokensMap, "/", TK_FSLASH);
    SCTM_put(singleCharTokensMap, "%", TK_PERCENT);
    SCTM_put(singleCharTokensMap, "^", TK_XOR);
    SCTM_put(singleCharTokensMap, "(", TK_LPAREN);
    SCTM_put(singleCharTokensMap, ")", TK_RPAREN);
    SCTM_put(singleCharTokensMap, "{", TK_LBRACE);
    SCTM_put(singleCharTokensMap, "}", TK_RBRACE);
    SCTM_put(singleCharTokensMap, ";", TK_SEMICOLON);
    SCTM_put(singleCharTokensMap, ",", TK_COMMA);
}

void populateLATM(void)
{
    lookAheadTokensMap = LATM_new(hashCStr, eqCStr, NULL, NULL);

    LATM_put(lookAheadTokensMap, "*", TK_ASTERISK);
    LATM_put(lookAheadTokensMap, "**", TK_EXP);
    LATM_put(lookAheadTokensMap, "=", TK_EQ);
    LATM_put(lookAheadTokensMap, "==", TK_EQEQ);
    LATM_put(lookAheadTokensMap, "<", TK_LT);
    LATM_put(lookAheadTokensMap, "<=", TK_LE);
    LATM_put(lookAheadTokensMap, ">", TK_GT);
    LATM_put(lookAheadTokensMap, ">=", TK_GE);
    LATM_put(lookAheadTokensMap, "!", TK_NOT);
    LATM_put(lookAheadTokensMap, "&&", TK_AND);
    LATM_put(lookAheadTokensMap, "||", TK_OR);
    LATM_put(lookAheadTokensMap, "!=", TK_NEQ);
}

void populateKTTKM(void)
{
    keywordsToTokenKindMap = KTTKM_new(hashCStr, eqCStr, NULL, NULL);

    KTTKM_put(keywordsToTokenKindMap, "true", TK_TRUE);
    KTTKM_put(keywordsToTokenKindMap, "false", TK_FALSE);
    KTTKM_put(keywordsToTokenKindMap, "if", TK_IF);
    KTTKM_put(keywordsToTokenKindMap, "unless", TK_UNLESS);
    KTTKM_put(keywordsToTokenKindMap, "elif", TK_ELIF);
    KTTKM_put(keywordsToTokenKindMap, "else", TK_ELSE);
    KTTKM_put(keywordsToTokenKindMap, "while", TK_WHILE);
    KTTKM_put(keywordsToTokenKindMap, "until", TK_UNTIL);
    KTTKM_put(keywordsToTokenKindMap, "forever", TK_FOREVER);
    KTTKM_put(keywordsToTokenKindMap, "repeat", TK_REPEAT);
    KTTKM_put(keywordsToTokenKindMap, "stop", TK_STOP);
    KTTKM_put(keywordsToTokenKindMap, "skip", TK_SKIP);
    KTTKM_put(keywordsToTokenKindMap, "onstop", TK_ONSTOP);
    KTTKM_put(keywordsToTokenKindMap, "nostop", TK_NOSTOP);
    KTTKM_put(keywordsToTokenKindMap, "function", TK_FUNCTION);
    KTTKM_put(keywordsToTokenKindMap, "return", TK_RETURN);
    KTTKM_put(keywordsToTokenKindMap, "returns", TK_RETURNS);
}

void populateTDM(void)
{
    tokenDisplayMap = TDM_new(hashTokenKind, eqTokenKind, NULL, NULL);

    TDM_put(tokenDisplayMap, TK_PLUS, "+");
    TDM_put(tokenDisplayMap, TK_MINUS, "-");
    TDM_put(tokenDisplayMap, TK_ASTERISK, "*");
    TDM_put(tokenDisplayMap, TK_EXP, "**");
    TDM_put(tokenDisplayMap, TK_FSLASH, "/");
    TDM_put(tokenDisplayMap, TK_PERCENT, "%");
    TDM_put(tokenDisplayMap, TK_LPAREN, "(");
    TDM_put(tokenDisplayMap, TK_RPAREN, ")");
    TDM_put(tokenDisplayMap, TK_LBRACE, "{");
    TDM_put(tokenDisplayMap, TK_RBRACE, "}");
    TDM_put(tokenDisplayMap, TK_SEMICOLON, ";");
    TDM_put(tokenDisplayMap, TK_EQ, "=");
    TDM_put(tokenDisplayMap, TK_LT, "<");
    TDM_put(tokenDisplayMap, TK_GT, ">");
    TDM_put(tokenDisplayMap, TK_EQEQ, "==");
    TDM_put(tokenDisplayMap, TK_NOT, "!");
    TDM_put(tokenDisplayMap, TK_AND, "&&");
    TDM_put(tokenDisplayMap, TK_OR, "||");
    TDM_put(tokenDisplayMap, TK_XOR, "^");
    TDM_put(tokenDisplayMap, TK_NEQ, "!=");
    TDM_put(tokenDisplayMap, TK_LE, "<=");
    TDM_put(tokenDisplayMap, TK_GE, ">=");
    TDM_put(tokenDisplayMap, TK_NUMBER, "number");
    TDM_put(tokenDisplayMap, TK_IDENTIFIER, "identifier");
    TDM_put(tokenDisplayMap, TK_TRUE, "true");
    TDM_put(tokenDisplayMap, TK_FALSE, "false");
    TDM_put(tokenDisplayMap, TK_IF, "if");
    TDM_put(tokenDisplayMap, TK_UNLESS, "unless");
    TDM_put(tokenDisplayMap, TK_ELIF, "elif");
    TDM_put(tokenDisplayMap, TK_ELSE, "else");
    TDM_put(tokenDisplayMap, TK_WHILE, "while");
    TDM_put(tokenDisplayMap, TK_FOREVER, "forever");
    TDM_put(tokenDisplayMap, TK_UNTIL, "until");
    TDM_put(tokenDisplayMap, TK_REPEAT, "repeat");
    TDM_put(tokenDisplayMap, TK_STOP, "stop");
    TDM_put(tokenDisplayMap, TK_SKIP, "skip");
    TDM_put(tokenDisplayMap, TK_ONSTOP, "onstop");
    TDM_put(tokenDisplayMap, TK_NOSTOP, "nostop");
    TDM_put(tokenDisplayMap, TK_FUNCTION, "function");
    TDM_put(tokenDisplayMap, TK_RETURN, "return");
    TDM_put(tokenDisplayMap, TK_RETURNS, "returns");
    TDM_put(tokenDisplayMap, TK_COMMA, ",");
    TDM_put(tokenDisplayMap, TK_EOF, "end of file");
}

void initTokenTables(void)
{
    populateSCTM();
    populateLATM();
    populateKTTKM();
    populateTDM();
}

void freeTokenTables(void)
{
    SCTM_free(singleCharTokensMap);
    LATM_free(lookAheadTokensMap);
    KTTKM_free(keywordsToTokenKindMap);
    TDM_free(tokenDisplayMap);

    singleCharTokensMap = NULL;
    lookAheadTokensMap = NULL;
    keywordsToTokenKindMap = NULL;
    tokenDisplayMap = NULL;
}