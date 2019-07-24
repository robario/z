%{
#include "./parser.h"

int yywrap(void) {
    return 1;
}

#if YYDEBUG
#define YY_USER_ACTION                            \
    do {                                          \
        yylloc.first_line = yylloc.last_line;     \
        yylloc.first_column = yylloc.last_column; \
        for (yy_size_t i = 0; i < yyleng; ++i) {  \
            if (yytext[i] == '\n') {              \
                yylloc.last_column = 0;           \
                ++yylloc.last_line;               \
            } else {                              \
                ++yylloc.last_column;             \
            }                                     \
        }                                         \
    } while (0)                                   \
        ;
#endif

static char delimiter;
%}
%option noinput
%option nounput
%x IN_STRING
%%
[\x22\x27] { delimiter = yytext[0]; BEGIN(IN_STRING); }
<IN_STRING><<EOF>> { YY_FATAL_ERROR("unexpected EOF"); }
<IN_STRING>[^\x22\x27\x5c]+ { yymore(); }
<IN_STRING>\x5c. { yymore(); }
<IN_STRING>[\x22\x27] {
    if (yytext[yyleng - 1] == delimiter) {
        yytext[yyleng - 1] = '\0';
        BEGIN(INITIAL);
        return (yylval = string(yytext, delimiter))->type;
    }
    yymore();
}
"function " { return FUNCTION; }
" * " { return MULTIPLY; }
" / " { return DIVIDE; }
" + " { return ADD; }
" - " { return SUBTRACT; }
" = " { return ASSIGN; }
", " { return ACCUMULABLE; }
"-" { return MINUS; }
";" { return SEQUENTIAL; }
"(" { return GROUP_BEGIN; }
")" { return GROUP_END; }
" {" { return BLOCK_BEGIN; }
"}" { return BLOCK_END; }
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
"."?[[:alpha:]][[:alnum:]]* { return (yylval = identifier(yytext))->type; }
. { return yytext[0]; }
