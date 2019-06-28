%{
#include "./parser.h"

int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
%%
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
. { return yytext[0]; }
