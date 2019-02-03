%{
#include "./parser.h"

int yywrap(void) {
    return 1;
}

static void yy_user_action(void) {
    yylloc.first_line = yylloc.last_line;
    yylloc.first_column = yylloc.last_column;
    if (*yytext == '\n') {
        yylloc.last_column = 0;
        ++yylloc.last_line;
    } else {
        yylloc.last_column += yyleng;
    }
}
#define YY_USER_ACTION yy_user_action();
%}
%option noinput
%option nounput
dotall (.|\n)
%%
" + " { return ADD; }
" - " { return SUBTRACT; }
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
{dotall} { return yytext[0]; }
