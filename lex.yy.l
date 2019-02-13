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
static char delimiter;
%}
%option noinput
%option nounput
dotall (.|\n)
%x IN_STRING
%%
[\x22\x27] { delimiter = yytext[0]; BEGIN(IN_STRING); }
<IN_STRING><<EOF>> { YY_FATAL_ERROR("unexpected EOF"); }
<IN_STRING>[^\x22\x27\x5c]+ { yymore(); }
<IN_STRING>\x5c{dotall} { yymore(); }
<IN_STRING>[\x22\x27] {
    if (yytext[yyleng - 1] == delimiter) {
        yytext[yyleng - 1] = '\0';
        BEGIN(INITIAL);
        return (yylval = string(yytext, delimiter))->type;
    }
    yymore();
}
"function" { return FUNCTION; }
" * " { return MULTIPLY; }
" / " { return DIVIDE; }
" + " { return ADD; }
" - " { return SUBTRACT; }
" == " { return EQ; }
" != " { return NE; }
" = " { return ASSIGN; }
", " { return COMMA; }
"-" { return UMINUS; }
";"\n*\t* { return SEPARATOR; }
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
[[:alpha:]][[:alnum:]]* { return (yylval = identifier(yytext))->type; }
{dotall} { return yytext[0]; }
