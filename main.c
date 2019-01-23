//
// @(#) main - z main
//
#include <stdlib.h>
#include "./parser.h"
#include "./generator.h"

int main(void) {
    if (parse() != 0) {
        return EXIT_FAILURE;
    }
    generate();
    return EXIT_SUCCESS;
}
