//
// @(#) main - z main
//
#include <stdlib.h>
#include "./parser.h"

int main(void) {
    if (parse() != 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
