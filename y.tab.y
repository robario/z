%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(YYSTYPE *yylval, const char *message) {
    (void)yylval;
    fprintf(stderr, "%s\n", message);
}
%}
%parse-param { YYSTYPE *ast }
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
%%
const char *enum_NodeType(NodeType type) {
    assert(0 <= type && type <= YYUNDEFTOK || YYERRCODE + YYUNDEFTOK <= type && type <= YYMAXUTOK);
    return yytname[YYTRANSLATE(type)];
}
