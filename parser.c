#include <stdlib.h>
#include "./parser.h"

int yyparse(Node **ast);

int parse(Node** ast) {
    return yyparse(ast);
}

static Node *new_node(NodeClass class, NodeType type, void *value, Node *left, Node *right) {
    Node *self = malloc(sizeof(Node));
    self->class = class;
    self->type = type;
    self->value = value;
    self->left = left;
    self->right = right;
    return self;
}

Node *program(Node *body) {
    return new_node(GENERAL_NODE, PROGRAM, body, NULL, NULL);
}

Node *binary(NodeType type, Node *left, Node *right) {
    return new_node(OPERATOR_NODE, type, NULL, left, right);
}

Node *unary(NodeType type, Node *operand) {
    return binary(type, operand, NULL);
}

Node *number(const char *yytext) {
    long long int *value = malloc(sizeof(long long int));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value, NULL, NULL);
}
