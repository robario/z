#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"

int yyparse(YYSTYPE *yylval);

static Node *table_list;
static Node *function_list;

void table_new(void) {
    assert(table_list);
    list_append(table_list, list_new());
    verbose("%s", node_string(table_list));
}

static Node *table_top(void) {
    assert(table_list);
    assert(ListValue(table_list)->size);
    verbose("%s", node_string(table_list));
    return ListValue(table_list)->nodes[ListValue(table_list)->size - 1];
}

static void table_restore(void) {
    assert(table_list);
    assert(ListValue(table_list)->size);
    --ListValue(table_list)->size;
    verbose("%s", node_string(table_list));
}

Node *parse(void) {
    function_list = list_new();
    table_list = list_new();
    table_new();
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
    value->table = table_top();
    value->body = body;
    table_restore();

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

Node *call(Node *operand) {
    assert(operand);
    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->table = NULL;
    value->body = operand;
    return new_node(VALUE_NODE, CALL, value);
}

Node *locator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node *node = list_find(table_top(), name, strlen(name) + 1);
    if (node == NULL) {
        node = identifier;
        node->type = LOCATOR;
        list_append(table_top(), node);
        verbose("%s into %s", node_string(node), node_string(table_top()));
    }
    return node;
}

Node *delocator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node *node = list_find(table_top(), name, strlen(name) + 1);
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
