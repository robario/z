#include <stdlib.h>
#include "./parser.h"

int yyparse(Node **ast);

int parse(Node** ast) {
    return yyparse(ast);
}

static Node *new_node(NodeClass class, NodeType type, void *value) {
    Node *self = malloc(sizeof(Node));
    self->class = class;
    self->type = type;
    self->value = value;
    return self;
}

Node *number(const char *yytext) {
    long long int *value = malloc(sizeof(long long int));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value);
}
