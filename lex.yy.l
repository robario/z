%{
#include "./y.tab.h"

int yywrap(void) {
    return 1;
}
%}
%option noinput
%option nounput
dotall (.|\n)
%%
[[:digit:]]|[1-9][[:digit:]]+ { yylval = (int)strtol(yytext, NULL, 10); return NUMBER; }
{dotall} { return yytext[0]; }
