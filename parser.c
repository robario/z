#include "./parser.h"

int yyparse(int *status);

int parse(int *status) {
    return yyparse(status);
}
