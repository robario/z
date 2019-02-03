%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(Node **ast, const char *message) {
    (void)ast;
    fprintf(stderr, "%s\n", message);
}

static void yyprint(FILE *yyoutput, int yytype, const YYSTYPE yyvalue) {
    switch (yytype) {
    case NUMBER:
        fprintf(yyoutput, "%lld", *(long long int *)yyvalue->value);
        break;
    }
}
#define YYPRINT(File, Type, Value) yyprint(File, Type, Value)
%}
%debug
%error-verbose
%locations
%no-lines
%token-table

%parse-param { Node **ast }
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
#if 2 <= DEBUG
int yydebug = 1;
#endif

const char *enum_yytokentype(int yytype) {
    return yytname[YYTRANSLATE(yytype)];
}
