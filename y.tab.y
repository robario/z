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
%token PROGRAM

%left SUBTRACT
%left ADD
%left DIVIDE
%left MULTIPLY
%right UMINUS

%token NUMBER
%%
program
        : error { YYABORT; }
        | void { *ast = program(number("0")); }
        | expression { *ast = program($1); }
        ;

expression
        : additive
        ;

additive
        : multiplicative
        | additive SUBTRACT multiplicative { $$ = binary(SUBTRACT, $1, $3); }
        | additive ADD multiplicative { $$ = binary(ADD, $1, $3); }
        ;

multiplicative
        : unary
        | multiplicative DIVIDE unary { $$ = binary(DIVIDE, $1, $3); }
        | multiplicative MULTIPLY unary { $$ = binary(MULTIPLY, $1, $3); }
        ;

unary
        : primary
        | UMINUS unary { $$ = unary(UMINUS, $2); }
        ;

primary
        : NUMBER
        | '(' expression ')' { $$ = $2; }
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
