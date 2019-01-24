#ifndef DEBUG_H
#define DEBUG_H

#ifndef DEBUG
#define DEBUG 0
#endif

#if ! DEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif
#include <assert.h>

#if 2 <= DEBUG
#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#else
#define YYDEBUG 0
#define YYERROR_VERBOSE 0
#endif

#define error(...)                              \
    do {                                        \
        fprintf(stderr, "error: " __VA_ARGS__); \
        fputc('\n', stderr);                    \
        exit(1);                                \
    } while (0)

#endif
