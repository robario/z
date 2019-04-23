#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#define YYSTYPE struct Node *
#include "./y.tab.h"

#define ENUM_DECLARE(T) typedef enum T { T() } T; const char *enum_##T(T type)
#define ENUM_DEFINE(T) const char *enum_##T(T type) { switch (type) { T() } }

#define ENUM_ID(ID) ID,

#define NodeClass()                \
    ENUM_ID(INCOMPLETE_NODE)       \
    ENUM_ID(GENERAL_NODE)          \
    ENUM_ID(ACCUMULABLE_LIST_NODE) \
    ENUM_ID(SEQUENTIAL_LIST_NODE)  \
    ENUM_ID(OPERATOR_NODE)         \
    ENUM_ID(VALUE_NODE)
ENUM_DECLARE(NodeClass);

#undef ENUM_ID
#define ENUM_ID(ID) case ID: return #ID;

typedef enum yytokentype NodeType;

typedef struct Node {
    NodeClass class;
    union {
        NodeType type;
        size_t index;
    };
    union {
        void *value;
        struct Node *node;
    };
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *function_list;
    Node *string_list;
    Node *body;
} ProgramValue;

typedef struct {
    Node *locator_table;
    Node *parameter_list;
    Node *body;
} FunctionValue;

const char *enum_yytokentype(int yytype);

int parse(Node **ast);

Node *new_accumulable_list(Node **list_head, Node *node);
Node *new_sequential_list(Node **list_head, Node *node);
Node *list_append(Node **list_head, Node *node);
Node *list_find_cell(Node *list_head, Node *node, bool equivalence);
size_t list_size(Node *list_head);

Node *program(Node *body);
void new_locator_table(void);
void restore_locator_table(void);
Node *function(Node *formal, Node *body);
Node *binary(NodeType type, Node *left, Node *right);
Node *unary(NodeType type, Node *operand);
Node *call(Node *delocator, Node *actual);
Node *delocator(Node *identifier);
Node *locator(Node *identifier);

Node *identifier(const char *yytext);
Node *string(const char *yytext, const char delimiter);
Node *number(const char *yytext);

#endif
