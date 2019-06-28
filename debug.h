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

#endif
