%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(Node **ast, const char *message) {
    (void)ast;
    fprintf(stderr, "%s\n", message);
}
%}
%parse-param { Node **ast }
%token NUMBER
%%
program
        : error { YYABORT; }
        | void { *ast = number("0"); }
        | NUMBER { *ast = $1; }
        ;

void
        :
        ;
