%{
#include "./parser.h"

int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
dotall (.|\n)
%%
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
{dotall} { return yytext[0]; }
