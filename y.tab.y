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
%right MINUS

%token NUMBER

%token SEQUENTIAL
%token GROUP_BEGIN
%token GROUP_END
%%
program
        : error { YYABORT; }
        | sequential_expression { $$ = program($1); *ast = $$; }
        ;

sequential_expression
        : void { $$ = list_new(); list_append($$, number("0")); }
        | expression { $$ = list_new(); list_append($$, $1); }
        | sequential_expression SEQUENTIAL expression { $$ = $1; list_append($$, $3); }
        | sequential_expression SEQUENTIAL void
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
        | MINUS unary { $$ = unary(MINUS, $2); }
        ;

primary
        : NUMBER
        | GROUP_BEGIN expression GROUP_END { $$ = $2; }
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
