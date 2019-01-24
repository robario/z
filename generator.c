#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"
#include "./generator.h"

extern FILE *yyout;

static const size_t SPSIZE = 64 / 8;

#if 1 <= DEBUG
#include <stdarg.h>

static void debug_indent(int level) {
    static int indent = 0;
    if (level < 0) {
        indent += level;
    }
    fputc('#', stderr);
    for (int i = 0; i < indent; ++i) {
        fputc('\t', stderr);
    }
    if (0 < level) {
        indent += level;
    }
}

#define debug(level, ...) (debug_indent(level), fprintf(stderr, __VA_ARGS__), fputc('\n', stderr))

static void mnemonic_printf(const char *format, ...) {
    static ssize_t sp = 0;

    va_list ap;
    va_start(ap, format);
    char *buffer = malloc(vsnprintf(NULL, 0, format, ap) + 1);
    va_end(ap);
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    fputs(buffer, stderr);

    ssize_t diff = 0;
    if (sp != 0 && strcmp(buffer, "ret") == 0) {
        fputs(" ; unbalanced stack\n", stderr);
        exit(1);
    } else if (strncmp(buffer, "push ", 5) == 0) {
        diff = -SPSIZE;
    } else if (strncmp(buffer, "pop ", 4) == 0) {
        diff = SPSIZE;
    } else if (sscanf(buffer + 4, "rsp, %ld", &diff) != 0) {
        if (strncmp(buffer, "sub ", 4) == 0) {
            diff = -diff;
        } else if (strncmp(buffer, "add ", 4) == 0) {
            diff = +diff;
        }
    }

    if (diff != 0) {
        sp += diff;
        fprintf(stderr, " \t; {%ld}", sp / -SPSIZE);
    }

    fputs(buffer, yyout);
}
#else
#define debug_indent(...) ((void)0)
#define debug(...) ((void)0)
#define mnemonic_printf(...) fprintf(yyout, __VA_ARGS__)
#endif

#define mnemonic_begin(...) (debug_indent(0), fputc('\t', yyout), mnemonic_printf(__VA_ARGS__))
#define mnemonic_end() mnemonic_printf("\n")
#define mnemonic(...) (mnemonic_begin(__VA_ARGS__), mnemonic_end())
#define label(...) (debug_indent(0), mnemonic_printf(__VA_ARGS__), mnemonic_printf("\n"))

ENUM_DEFINE(NodeClass);

void generate(Node *node) {
    debug(+1, "<%s, %s> {", enum_NodeClass(node->class), enum_yytokentype(node->type));

    mnemonic(".intel_syntax noprefix");
    mnemonic(".text");
    mnemonic(".global _start");
    label("_start:");
    switch (node->class) {
    case VALUE_NODE:
        switch (node->type) {
        case NUMBER:
            mnemonic("mov rax, %lld", *(long long int *)node->value);
            break;
        }
        mnemonic("push rax");
        break;
    }
    mnemonic("pop rax");
    mnemonic("ret");

    debug(-1, "}");
}
