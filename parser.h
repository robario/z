#ifndef PARSER_H
#define PARSER_H

#include "./node.h"

Node *parse(void);

Node *program(Node *body);
Node *function(Node *body);
Node *binary(NodeType type, Node *lhs, Node *rhs);
Node *unary(NodeType type, Node *operand);
Node *locator(Node *identifier);
Node *delocator(Node *identifier);

Node *identifier(const char *yytext);
Node *number(const char *yytext);

#endif
