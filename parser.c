#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"

int yyparse(YYSTYPE *yylval);

static Node *table;
static Node *function_list;

Node *parse(void) {
    function_list = list_new();
    table = list_new();
    Node *ast;
    if (yyparse(&ast) != 0) {
        return NULL;
    }
    return ast;
}

Node *program(Node *body) {
    assert(body);
    ProgramValue *value = malloc(sizeof(ProgramValue));
    value->function_list = function_list;
    value->body = function(body);
    return new_node(GENERAL_NODE, PROGRAM, value);
}

Node *function(Node *body) {
    assert(body);
    assert(body->class == LIST_NODE && body->type == SEQUENTIAL);

    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->table = table;
    value->body = body;

    Node *function = new_node(VALUE_NODE, FUNCTION, value);
    list_append(function_list, function);
    return function;
}

Node *binary(NodeType type, Node *lhs, Node *rhs) {
    assert(enum_NodeType(type));
    assert(lhs);
    OperatorValue *value = malloc(sizeof(OperatorValue));
    value->lhs = lhs;
    value->rhs = rhs;
    return new_node(OPERATOR_NODE, type, value);
}

Node *unary(NodeType type, Node *operand) {
    assert(enum_NodeType(type));
    assert(operand);
    return binary(type, operand, NULL);
}

Node *locator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node *node = list_find(table, name, strlen(name) + 1);
    if (node == NULL) {
        node = identifier;
        node->type = LOCATOR;
        list_append(table, node);
        verbose("%s into %s", node_string(node), node_string(table));
    }
    return node;
}

Node *delocator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node *node = list_find(table, name, strlen(name) + 1);
    if (node == NULL) {
        error("use of undeclared identifier %s", node_string(identifier));
    }
    return new_node(VALUE_NODE, DELOCATOR, node);
}

Node *identifier(const char *yytext) {
    assert(strlen(yytext));
    char *value = strdup(yytext);
    return new_node(VALUE_NODE, IDENTIFIER, value);
}

Node *number(const char *yytext) {
    assert(strlen(yytext));
    NumberValue *value = malloc(sizeof(NumberValue));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value);
}
