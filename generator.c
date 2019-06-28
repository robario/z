#include <stdio.h>
#include "./generator.h"

extern FILE *yyout;

#define mnemonic(...)                \
    do {                             \
        fputc('\t', yyout);          \
        fprintf(yyout, __VA_ARGS__); \
        fputc('\n', yyout);          \
    } while (0)

#define label(...)                   \
    do {                             \
        fprintf(yyout, __VA_ARGS__); \
        fputc('\n', yyout);          \
    } while (0)

void generate(Node *node) {
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
}
