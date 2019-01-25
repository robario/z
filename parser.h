#ifndef PARSER_H
#define PARSER_H

#define YYSTYPE struct Node *
#include "./y.tab.h"

#define ENUM_DECLARE(T) typedef enum T { T() } T; const char *enum_##T(T type)
#define ENUM_DEFINE(T) const char *enum_##T(T type) { switch (type) { T() } }

#define ENUM_ID(ID) ID,

#define NodeClass()        \
    ENUM_ID(GENERAL_NODE)  \
    ENUM_ID(OPERATOR_NODE) \
    ENUM_ID(VALUE_NODE)
ENUM_DECLARE(NodeClass);

#undef ENUM_ID
#define ENUM_ID(ID) case ID: return #ID;

typedef enum yytokentype NodeType;

typedef struct Node {
    NodeClass class;
    NodeType type;
    void *value;
    struct Node *left;
    struct Node *right;
} Node;

const char *enum_yytokentype(int yytype);

int parse(Node **ast);

Node *program(Node *body);
Node *binary(NodeType type, Node *left, Node *right);
Node *unary(NodeType type, Node *operand);
Node *number(const char *yytext);

#endif
