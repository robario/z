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
%token FUNCTION
%token CALL

%right ASSIGN
%left SUBTRACT
%left ADD
%left DIVIDE
%left MULTIPLY
%right MINUS

%token LOCATOR
%token DELOCATOR
%token IDENTIFIER
%token STRING
%token NUMBER

%token ACCUMULABLE
%token SEQUENTIAL
%token GROUP_BEGIN
%token GROUP_END
%token BLOCK_BEGIN
%token BLOCK_END
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
        : assignable
        ;

assignable
        : additive
        | locator ASSIGN assignable { $$ = binary(ASSIGN, $1, $3); }
        | FUNCTION { table_new(); } GROUP_BEGIN formal GROUP_END BLOCK_BEGIN sequential_expression BLOCK_END { $$ = function($4, $7, NULL); }
        | FUNCTION { table_new(); } IDENTIFIER GROUP_BEGIN formal GROUP_END BLOCK_BEGIN sequential_expression BLOCK_END { $$ = function($5, $8, $3); }
        ;
formal
        : void { $$ = list_new_accumulable(); }
        | locator { $$ = list_new_accumulable(); list_append($$, $1); }
        | formal ACCUMULABLE locator { $$ = $1; list_append($$, $3); }
        ;

locator
        : IDENTIFIER { $$ = locator($1); }
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
        : postfix
        | MINUS unary { $$ = unary(MINUS, $2); }
        ;

postfix
        : primary
        | postfix GROUP_BEGIN actual GROUP_END { $$ = call($1, $3); }
        ;

actual
        : void { $$ = list_new_accumulable(); }
        | expression { $$ = list_new_accumulable(); list_append($$, $1); }
        | actual ACCUMULABLE expression { $$ = $1; list_append($$, $3); }
        ;

primary
        : NUMBER
        | STRING
        | delocator
        | GROUP_BEGIN expression GROUP_END { $$ = $2; }
        ;

delocator
        : IDENTIFIER { $$ = delocator($1); }
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
