#ifndef NODE_H
#define NODE_H

#include "./debug.h"
#define YYSTYPE struct Node *
#include "./y.tab.h"

#define ENUM_TAG(tag) tag,
#define ENUM_DEFINE(T) typedef enum T { T() } T; const char *enum_##T(T type)

#define NodeClass()         \
    ENUM_TAG(GENERAL_NODE)  \
    ENUM_TAG(LIST_NODE)     \
    ENUM_TAG(OPERATOR_NODE) \
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

#define StringValue(node) ((char *)(node)->value)

typedef struct {
    Node *lhs;
    Node *rhs;
} OperatorValue;
#define OperatorValue(node) ((OperatorValue *)(node)->value)

typedef struct {
    size_t size;
    Node **nodes;
    size_t capacity;
} ListValue;
#define ListValue(node) ((ListValue *)(node)->value)

typedef struct {
    Node *parameter_list;
    Node *table;
    Node *body;
} FunctionValue;
#define FunctionValue(node) ((FunctionValue *)(node)->value)

typedef struct {
    const char *name;
    Node *node;
} GlobalValue;
#define GlobalValue(node) ((GlobalValue *)(node)->value)

typedef struct {
    Node *function_list;
    Node *string_list;
    Node *global_list;
    Node *body;
} ProgramValue;
#define ProgramValue(node) ((ProgramValue *)(node)->value)

Node *new_node(NodeClass class, NodeType type, void *value);

Node *list_new(void);
Node *list_new_accumulable(void);
void list_append(Node *list, Node *node);
size_t list_index(Node *list, Node *node);
Node *list_find(Node *list, const void *value, size_t size);

char *node_class_string(Node *node);
char *node_string(Node *node);

#define is_global(node) (StringValue((node))[0] == '.')
#define is_extern(node) (is_global((node)) && strrchr(StringValue((node)), '.') != StringValue((node)))

#endif
