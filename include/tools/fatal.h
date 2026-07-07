#ifndef TOOLS_FATAL_H_
#define TOOLS_FATAL_H_

// Prints "Fatal: " followed by the formatted message to stderr,
// then aborts. Never returns.
//
// Usage:
//   SparrowFatal("malloc(%zu) failed", size);
//   SparrowFatal("integer overflow: %s + %s", "a", "b");
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noreturn, format(printf, 1, 2)))
#endif
void SparrowFatal(const char *fmt, ...);

#endif