#include <stdlib.h>
#include <string.h>
#include "./parser.h"

int yyparse(YYSTYPE *yylval);

Node *parse(void) {
    Node *ast;
    if (yyparse(&ast) != 0) {
        return NULL;
    }
    return ast;
}

Node *program(Node *body) {
    return new_node(GENERAL_NODE, PROGRAM, body);
}

Node *number(const char *yytext) {
    assert(strlen(yytext));
    NumberValue *value = malloc(sizeof(NumberValue));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value);
}
