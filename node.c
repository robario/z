#include <stdlib.h>
#include "./node.h"

ENUM_DEFINE(NodeClass);

Node *new_node(NodeClass class, NodeType type, void *value) {
    assert(enum_NodeClass(class));
    assert(enum_NodeType(type));
    assert(value);
    Node *node = malloc(sizeof(Node));
    node->class = class;
    node->type = type;
    node->value = value;
    return node;
}
