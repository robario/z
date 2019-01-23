#include <stdio.h>
#include "./generator.h"

extern FILE *yyout;

#define mnemonic(...) (fputc('\t', yyout), fprintf(yyout, __VA_ARGS__), fputc('\n', yyout))
#define label(...) (fprintf(yyout, __VA_ARGS__), fputc('\n', yyout))

void generate(void) {
    mnemonic(".intel_syntax noprefix");
    mnemonic(".text");
    mnemonic(".global _start");
    label("_start:");
    mnemonic("mov rax, 0");
    mnemonic("ret");
}