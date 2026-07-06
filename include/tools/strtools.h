#ifndef TOOLS_STRCLONE_H
#define TOOLS_STRCLONE_H

#include <stddef.h>

size_t SparrowStrLen(const char *s, size_t maxlen);
char *SparrowStrClone(const char *str, size_t n);

#endif