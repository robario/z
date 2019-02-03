#include <stdio.h>
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

char *node_class_string(Node *node) {
    assert(node);
    char *string = NULL;
    asprintf(&string, "<%s:%s>", enum_NodeClass(node->class), enum_NodeType(node->type));
    return string;
}

char *node_string(Node *node) {
    char *string;
    size_t size;
    FILE *memout = open_memstream(&string, &size);
    switch (node->type) {
    case NUMBER:
        fprintf(memout, "%lld", NumberValue(node));
        break;
    default:
        fprintf(memout, "#0x%08llx", (unsigned long long int)node & 0x00000000FFFFFFFF);
        break;
    }
    fclose(memout);
    return string;
}
