#ifndef PARSER_H
#define PARSER_H

#include "./node.h"

void table_new(void);
Node *parse(void);

void table_new(void);

Node *program(Node *body);
Node *function(Node *formal, Node *body);
Node *binary(NodeType type, Node *lhs, Node *rhs);
Node *unary(NodeType type, Node *operand);
Node *call(Node *operand, Node *actual);
Node *locator(Node *identifier);
Node *delocator(Node *identifier);

Node *identifier(const char *yytext);
Node *string(const char *yytext, char delimiter);
Node *number(const char *yytext);

#endif
