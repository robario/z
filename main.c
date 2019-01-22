//
// @(#) main - z main
//
#include <stdlib.h>
#include "./parser.h"
#include "./generator.h"

int main(void) {
    int status;
    if (parse(&status) != 0) {
        return EXIT_FAILURE;
    }
    generate(status);
    return EXIT_SUCCESS;
}
