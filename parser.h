#ifndef PARSER_H
#define PARSER_H

typedef long long int NumberValue;

#define YYSTYPE NumberValue
#include "./y.tab.h"

int parse(NumberValue *status);

#endif
