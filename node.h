#ifndef NODE_H
#define NODE_H

#include "./debug.h"
#define YYSTYPE struct Node *
#include "./y.tab.h"

#define ENUM_TAG(tag) tag,
#define ENUM_DEFINE(T) typedef enum T { T() } T; const char *enum_##T(T type)

#define NodeClass()        \
    ENUM_TAG(GENERAL_NODE) \
    ENUM_TAG(VALUE_NODE)
ENUM_DEFINE(NodeClass);

#undef ENUM_TAG
#undef ENUM_DEFINE
#define ENUM_TAG(tag) case tag: return #tag;
#define ENUM_DEFINE(T) const char *enum_##T(T type) { switch (type) { T() } return NULL; }

typedef enum yytokentype NodeType;
const char *enum_NodeType(NodeType type);

typedef struct Node {
    NodeClass class;
    NodeType type;
    void *value;
} Node;

#define NodeValue(node) ((Node *)(node)->value)

typedef long long int NumberValue;
#define NumberValue(node) *((NumberValue *)(node)->value)

Node *new_node(NodeClass class, NodeType type, void *value);

char *node_class_string(Node *node);
char *node_string(Node *node);

#endif
