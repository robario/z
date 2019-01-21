#include "./parser.h"

int yyparse(void);

int parse(void) {
    return yyparse();
}
