%{
#include <stdio.h>
#include "./parser.h"

int yylex(void);

void yyerror(Node **ast, const char *message) {
    (void)ast;
    fprintf(stderr, "%s\n", message);
}

void yyprint(FILE *yyoutput, enum yytokentype yytype, const YYSTYPE yyvalue) {
    switch (yytype) {
    case NUMBER:
        fprintf(yyoutput, "%lld", *(long long int *)yyvalue->value);
        break;
    case STRING:
        fputc('"', yyoutput);
        for (const char *c = yyvalue->value; *c != '\0'; ++c) {
            if (*c < 20) {
                fprintf(yyoutput, "\x5cx%02x", *c);
            } else {
                fputc(*c, yyoutput);
            }
        }
        fputc('"', yyoutput);
        break;
    case IDENTIFIER:
    case LOCATOR:
        fprintf(yyoutput, "'%s'", yyvalue->value);
        break;
    case DELOCATOR:
        fprintf(yyoutput, "'*%s'", ((Node *)yyvalue->value)->value);
        break;
    default:
        fprintf(yyoutput, "0x%08llx", (unsigned long long int)yyvalue & 0x00000000FFFFFFFF);
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
%token FUNCTION
%token CALL

%left SEPARATOR
%left COMMA
%right ASSIGN
%left SUBTRACT
%left ADD
%left DIVIDE
%left MULTIPLY
%right UMINUS

%token DELOCATOR
%token LOCATOR
%token IDENTIFIER
%token STRING
%token NUMBER
%%
program
        : error { YYABORT; }
        | compound { *ast = program(function(NULL, $1)); }
        ;

expression
        : assignable
        ;

assignable
        : additive
        | locator ASSIGN assignable { $$ = binary(ASSIGN, $1, $3); }
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
        | STRING
        | delocator
        | function '(' formal ')' '{' compound '}'  { $$ = function($3, $6); }
        | delocator '(' accumulable ')' { $$ = call($1, $3); }
        | '(' sequential ')' { $$ = $2; }
        ;

function
        : FUNCTION { new_locator_table(); }
        ;

formal
        : void
        | formal_list
        ;

formal_list
        : locator { new_accumulable_list(&$$, $1); }
        | formal_list COMMA locator { list_append(&$1, $3); }
        ;

compound
        : void
        | compound_list
        ;

compound_list
        : expression { new_sequential_list(&$$, $1); }
        | compound SEPARATOR expression { list_append(&$1, $3); }
        | compound SEPARATOR void
        ;

accumulable
        : void
        | accumulable_list
        ;

accumulable_list
        : expression { new_accumulable_list(&$$, $1); }
        | accumulable_list COMMA expression { list_append(&$1, $3); }
        ;

sequential
        : sequential_list
        ;

sequential_list
        : expression { new_sequential_list(&$$, $1); }
        | sequential_list COMMA expression { list_append(&$1, $3); }
        ;

delocator
        : IDENTIFIER { $$ = delocator($1); }
        ;

locator
        : IDENTIFIER { $$ = locator($1); }
        ;

void
        : { $$ = NULL; }
        ;
%%
#if 2 <= DEBUG
int yydebug = 1;
#endif

const char *enum_yytokentype(int yytype) {
    return yytname[YYTRANSLATE(yytype)];
}
