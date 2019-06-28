//
// @(#) main - z main
//
#include <stdlib.h>
#include "./parser.h"
#include "./generator.h"

int main(void) {
    Node *ast = parse();
    if (ast == NULL) {
        return EXIT_FAILURE;
    }
    generate(ast);
    return EXIT_SUCCESS;
}
