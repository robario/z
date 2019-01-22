#include "./parser.h"

int yyparse(YYSTYPE *yylval);

int parse(NumberValue *status) {
    return yyparse(status);
}
