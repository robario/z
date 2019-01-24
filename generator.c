#include <stdio.h>
#include "./parser.h"
#include "./generator.h"

extern FILE *yyout;

#define mnemonic(...) (fputc('\t', yyout), fprintf(yyout, __VA_ARGS__), fputc('\n', yyout))
#define label(...) (fprintf(yyout, __VA_ARGS__), fputc('\n', yyout))

void generate(Node *node) {
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
        break;
    }
    mnemonic("ret");
}
