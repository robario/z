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

Node *list_new(void) {
    ListValue *value = malloc(sizeof(ListValue));
    value->size = 0;
    value->capacity = 8;
    value->nodes = malloc(sizeof(Node) * value->capacity);
    return new_node(LIST_NODE, SEQUENTIAL, value);
}

void list_append(Node *list, Node *node) {
    assert(list != NULL && list->class == LIST_NODE);
    assert(node);
    ListValue *value = ListValue(list);
    ++value->size;
    if (value->capacity < value->size) {
        value->capacity *= 2;
        verbose("realloc nodes to %zu", value->capacity);
        void *nodes = realloc(value->nodes, sizeof(Node) * value->capacity);
        assert(nodes);
        value->nodes = nodes;
    }
    value->nodes[value->size - 1] = node;
    verbose("%s", node_string(list));
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
    case SEQUENTIAL:
        fputc('[', memout);
        for (size_t index = 0; index < ListValue(node)->size; ++index) {
            if (index != 0) {
                fputc(',', memout);
            }
            fputs(node_string(ListValue(node)->nodes[index]), memout);
        }
        fputc(']', memout);
        break;
    default:
        fprintf(memout, "#0x%08llx", (unsigned long long int)node & 0x00000000FFFFFFFF);
        break;
    }
    fclose(memout);
    return string;
}
