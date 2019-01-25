%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(YYSTYPE *yylval, const char *message) {
    (void)yylval;
    fprintf(stderr, "%s\n", message);
}

#define YYPRINT(File, Type, Value) fputs((Value) && (Value)->type == Type ? node_string(Value) : enum_NodeType(Type), File)
%}
%parse-param { YYSTYPE *ast }
%token PROGRAM

%left SUBTRACT
%left ADD
%left DIVIDE
%left MULTIPLY

%token NUMBER
%%
program
        : error { YYABORT; }
        | void { *ast = program(number("0")); }
        | additive { *ast = program($1); }
        ;

additive
        : multiplicative
        | additive SUBTRACT multiplicative { $$ = binary(SUBTRACT, $1, $3); }
        | additive ADD multiplicative { $$ = binary(ADD, $1, $3); }
        ;

multiplicative
        : NUMBER
        | multiplicative DIVIDE NUMBER { $$ = binary(DIVIDE, $1, $3); }
        | multiplicative MULTIPLY NUMBER { $$ = binary(MULTIPLY, $1, $3); }
        ;

void
        :
        ;
%%
#if YYDEBUG
int yydebug = 1;
#endif

const char *enum_NodeType(NodeType type) {
    assert(0 <= type && type <= YYUNDEFTOK || YYERRCODE + YYUNDEFTOK <= type && type <= YYMAXUTOK);
    (void)yytoknum;
    return yytname[YYTRANSLATE(type)];
}
