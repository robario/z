%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(const char *message) {
    fprintf(stderr, "%s\n", message);
}
%}
%%
program
        : error { YYABORT; }
        | void
        ;

void
        :
        ;
