#include <stdio.h>
#include "./generator.h"

extern FILE *yyout;

#if DEBUG
#include <stdarg.h>
#include <string.h>

static void debug_printf(const char *format, ...) {
    static size_t indent = 0;
    int level = 0;
    char *p = strpbrk(format, "{}");
    if (p != NULL) {
        level = -(*p - 0x7c);
    }
    if (0 < level) {
        indent += level;
    }
    fputc('#', stderr);
    for (size_t i = 1; i < indent; ++i) {
        fputc('\t', stderr);
    }
    if (level < 0) {
        indent += level;
    }

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

static void mnemonic_printf(const char *format, ...) {
    char *buffer;
    va_list ap;
    va_start(ap, format);
    vasprintf(&buffer, format, ap);
    va_end(ap);

    fputs(buffer, stderr);
    fputs(buffer, yyout);
}
#else
#define debug_printf(...) ((void)0)
#define mnemonic_printf(...) fprintf(yyout, __VA_ARGS__)
#endif

#define mnemonic_indent() mnemonic_printf("\t")
#define mnemonic_begin()   \
    do {                   \
        debug_printf("");  \
        mnemonic_indent(); \
    } while (0)
#define mnemonic_end() mnemonic_printf("\n")

#define mnemonic(...)                 \
    do {                              \
        mnemonic_begin();             \
        mnemonic_printf(__VA_ARGS__); \
        mnemonic_end();               \
    } while (0)

#define label(...)                    \
    do {                              \
        debug_printf("");             \
        mnemonic_printf(__VA_ARGS__); \
        mnemonic_end();               \
    } while (0)

#define debug(format, ...) debug_printf(format "\n", ##__VA_ARGS__)

void generate(Node *node) {
    debug("%s%s {", node_class_string(node), node_string(node));

    mnemonic(".intel_syntax noprefix");
    mnemonic(".text");
    mnemonic(".global _start");
    label("_start:");
    switch (node->class) {
    case VALUE_NODE:
        switch (node->type) {
        case NUMBER:
            mnemonic("mov rax, %lld", NumberValue(node));
            break;
        }
        break;
    }
    mnemonic("ret");

    debug("}");
}
