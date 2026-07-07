#include "token.h"
#include "tools/allocator.h"
#include "tools/strtools.h"
#include "tools/ckdarith.h"

#include <stdlib.h>
#include <stdint.h>

Token newToken(TokenKind kind, char *value, Span span)
{
    uint64_t spanSub;
    SPARROW_CHECKED_SUB(span.end, span.start, &spanSub);

    return (Token){
        .kind = kind,
        .value = SparrowStrClone(value, spanSub),
        .span = span,
        .isInit = true,
    };
}

void freeToken(Token *token)
{
    free(token->value);
    token->value = NULL;
    token->isInit = false;
}
