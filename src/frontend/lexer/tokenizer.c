#include "frontend/lexer/tokenizer.h"
#include "frontend/lexer/tokens/tokenkind.h"

#include "tools/strtools.h"

#include <stdbool.h>

static inline bool isWhitespace(char c)
{
    return c == ' ' ||
           c == '\t' ||
           c == '\n' ||
           c == '\r' ||
           c == '\f' ||
           c == '\v';
}

static inline bool isIdentifierStart(char c)
{
    return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static inline bool isIdentifier(char c)
{
    return isIdentifierStart(c) || ('0' <= c && c <= '9');
}

static void tokenDtor(Token *t)
{
    freeToken(t);
}

TokenArray *tokenize(const char *src, size_t srcLen)
{
    TokenArray *tokArr = TokenArray_new(tokenDtor);

    size_t cursor = 0;

    while (cursor < srcLen)
    {
        char c = src[cursor];

        // skip whitespace
        if (isWhitespace(c))
        {
            cursor++;
            continue;
        }

        // skip comments
        if (c == '#')
        {
            while (cursor < srcLen && src[cursor] != '\n')
                cursor++;

            continue;
        }

        // handle multi digit numbers
        else if ('0' <= c && c <= '9')
        {
            size_t start = cursor;

            while (cursor < srcLen && '0' <= src[cursor] && src[cursor] <= '9')
                cursor++;

            TokenArray_push(tokArr, (Token){
                                        TK_NUMBER,
                                        SparrowStrClone(src + start, cursor - start),
                                        (Span){start, cursor},
                                    });
        }

        // handle identifiers
        else if (isIdentifierStart(c))
        {
            size_t start = cursor;

            while (cursor < srcLen && isIdentifier(src[cursor]))
                cursor++;

            char *identifier = SparrowStrClone(src + start, cursor - start);

            if (KTTKM_contains(keywordsToTokenKindMap, identifier))
                TokenArray_push(tokArr, (Token){
                                            KTTKM_get(keywordsToTokenKindMap, identifier),
                                            identifier,
                                            (Span){start, cursor},
                                        });
            else
                TokenArray_push(tokArr, (Token){
                                            TK_IDENTIFIER,
                                            identifier,
                                            (Span){start, cursor},
                                        });
        }

        // handle multi-char tokens
        else if (LATM_contains(lookAheadTokensMap, c))
        {
            char *unit = cursor + 1 < srcLen ? SparrowStrClone(src + cursor, 2) : c;

            if (LATM_contains(lookAheadTokensMap, unit))
            {
                TokenArray_push(tokArr, (Token){
                                            LATM_get(lookAheadTokensMap, unit),
                                            unit,
                                            (Span){cursor, cursor + 2},
                                        });
                cursor += 2;
            }
            else
            {
                TokenArray_push(tokArr, (Token){
                                            LATM_get(lookAheadTokensMap, c),
                                            c,
                                            (Span){cursor, cursor + 1},
                                        });
                cursor++;
            }
        }

        // handle single-char tokens
        else if (SCTM_contains(singleCharTokensMap, c))
        {
            TokenKind kind = SCTM_get(singleCharTokensMap, c);
            TokenArray_push(tokArr, (Token){
                                        kind,
                                        c,
                                        (Span){cursor, cursor + 1},
                                    });
            cursor++;
        }

        // handle unrecognized characters
        else
            abort(); // for now
    }

    TokenArray_push(tokArr, (Token){
                                TK_EOF,
                                NULL,
                                (Span){cursor, cursor},
                            });

    return tokArr;
}