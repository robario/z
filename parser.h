#ifndef PARSER_H
#define PARSER_H

#include "./node.h"

Node *parse(void);

Node *number(const char *yytext);

#endif
