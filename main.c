//
// @(#) main - z main
//
#include <stdio.h>
#include <stdlib.h>
#include "./parser.h"
#include "./generator.h"

extern FILE *yyin;

int main(int argc, char *argv[]) {
    if (2 <= argc) {
        yyin = fopen(argv[1], "r");
    }
    Node *ast;
    if (parse(&ast) != 0) {
        return EXIT_FAILURE;
    }
    generate(ast);
    fclose(yyin);
    return EXIT_SUCCESS;
}
