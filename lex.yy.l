%{
#include "./parser.h"

int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
%%
[[:digit:]]|[1-9][[:digit:]]+ { yylval = strtoll(yytext, NULL, 10); return NUMBER; }
. { return yytext[0]; }
