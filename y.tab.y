%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(int *status, const char *message) {
    (void)status;
    fprintf(stderr, "%s\n", message);
}
%}
%parse-param { int *status }
%token NUMBER
%%
program
        : error { YYABORT; }
        | void { *status = 0; }
        | NUMBER { *status = $1; }
        ;

void
        :
        ;
