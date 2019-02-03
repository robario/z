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
%}
%option noinput
%option nounput
%%
" + " { return ADD; }
" - " { return SUBTRACT; }
[[:digit:]]|[1-9][[:digit:]]+ { return (yylval = number(yytext))->type; }
. { return yytext[0]; }
