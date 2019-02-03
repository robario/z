#ifndef PARSER_H
#define PARSER_H

#include "./node.h"

Node *parse(void);

Node *program(Node *body);

Node *number(const char *yytext);

#endif
