%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(YYSTYPE *yylval, const char *message) {
    (void)yylval;
    fprintf(stderr, "%s\n", message);
}
%}
%parse-param { YYSTYPE *status }
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
