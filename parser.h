#ifndef PARSER_H
#define PARSER_H

#define YYSTYPE struct Node *
#include "./y.tab.h"

#define ENUM_DECLARE(T) typedef enum T { T() } T
#define ENUM_ID(ID) ID,

#define NodeClass() \
    ENUM_ID(VALUE_NODE)
ENUM_DECLARE(NodeClass);

typedef enum yytokentype NodeType;

typedef struct Node {
    NodeClass class;
    NodeType type;
    void *value;
} Node;

int parse(Node **ast);

Node *number(const char *yytext);

#endif
