#include "token.h"
#include "tools/allocator.h"
#include "tools/strtools.h"

#include <stdlib.h>

Token newToken(TokenKind kind, char *value, Span span)
{
    return (Token){
        .kind = kind,
        .value = SparrowStrClone(value, span.start - span.end),
        .span = span,
        .isInit = true,
    };
}

void freeToken(Token *token)
{
    free(token->value);
}
