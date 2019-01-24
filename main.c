//
// @(#) main - z main
//
#include <stdlib.h>
#include "./parser.h"
#include "./generator.h"

int main(void) {
    Node *ast;
    if (parse(&ast) != 0) {
        return EXIT_FAILURE;
    }
    generate(ast);
    return EXIT_SUCCESS;
}
