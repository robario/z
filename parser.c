#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"

int yyparse(YYSTYPE *yylval);

static Node *table_list;
static Node *function_list;
static Node *string_list;
static Node *global_list;

void table_new(void) {
    assert(table_list);
    list_append(table_list, list_new());
    verbose("%s", node_string(table_list));
}

static Node *table_top(void) {
    assert(table_list);
    assert(ListValue(table_list)->size);
    verbose("%s", node_string(table_list));
    return ListValue(table_list)->nodes[ListValue(table_list)->size - 1];
}

static void table_restore(void) {
    assert(table_list);
    assert(ListValue(table_list)->size);
    --ListValue(table_list)->size;
    verbose("%s", node_string(table_list));
}

Node *parse(void) {
    function_list = list_new();
    string_list = list_new();
    global_list = list_new();
    table_list = list_new();
    table_new();
    Node *ast;
    if (yyparse(&ast) != 0) {
        return NULL;
    }
    return ast;
}

Node *program(Node *body) {
    assert(body);
    ProgramValue *value = malloc(sizeof(ProgramValue));
    value->function_list = function_list;
    value->string_list = string_list;
    value->global_list = global_list;
    value->body = function(list_new_accumulable(), body, identifier("start"));
    return new_node(GENERAL_NODE, PROGRAM, value);
}

Node *function(Node *formal, Node *body, Node *identifier) {
    assert(formal);
    assert(body);
    assert(body->class == LIST_NODE && body->type == SEQUENTIAL);

    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->parameter_list = formal;
    value->table = table_top();
    value->body = body;
    table_restore();
    Node *function = new_node(VALUE_NODE, FUNCTION, value);
    list_append(function_list, function);
    if (identifier == NULL) {
        asprintf(&value->name, "function.%zu", ListValue(function_list)->size);
    } else {
        asprintf(&value->name, "_%s", StringValue(identifier));
    }
    return function;
}

Node *binary(NodeType type, Node *lhs, Node *rhs) {
    assert(enum_NodeType(type));
    assert(lhs);
    OperatorValue *value = malloc(sizeof(OperatorValue));
    value->lhs = lhs;
    value->rhs = rhs;
    return new_node(OPERATOR_NODE, type, value);
}

Node *unary(NodeType type, Node *operand) {
    assert(enum_NodeType(type));
    assert(operand);
    return binary(type, operand, NULL);
}

Node *call(Node *operand, Node *actual) {
    assert(operand);
    assert(actual);
    assert(actual->class == LIST_NODE && actual->type == ACCUMULABLE);
    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->parameter_list = actual;
    value->table = NULL;
    value->body = operand;
    value->name = NULL;
    return new_node(VALUE_NODE, CALL, value);
}

Node *locator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node* node = list_find(table_top(), name, strlen(name) + sizeof(char) * 1);
    if (node == NULL) {
        node = identifier;
        node->type = LOCATOR;
        list_append(table_top(), node);
        verbose("%s into %s", node_string(node), node_string(table_top()));
    }
    return node;
}

Node *delocator(Node *identifier) {
    assert(identifier);
    assert(identifier->type == IDENTIFIER);
    const char *name = StringValue(identifier);
    Node *node = list_find(table_top(), name, strlen(name) + sizeof(char));
    if (node == NULL) {
        node = identifier;
        node->type = LOCATOR;
    }
    return new_node(VALUE_NODE, DELOCATOR, node);
}

Node *identifier(const char *yytext) {
    assert(strlen(yytext));
    char *value = strdup(yytext);
    return new_node(VALUE_NODE, IDENTIFIER, value);
}

Node *string(const char *yytext, const char delimiter) {
    assert(yytext);
    assert(delimiter == 0x22 || delimiter == 0x27);
    char *value = strdup(yytext);
    if (delimiter == 0x22) {
        char *dest = value;
        for (const char *src = yytext; *src != '\0'; ++src, ++dest) {
            if (*src == 0x5c) {
                ++src;
                switch (*src) {
                case 'a': *dest = '\a'; break;
                case 'b': *dest = '\b'; break;
                case 'e': *dest = '\e'; break;
                case 'f': *dest = '\f'; break;
                case 'n': *dest = '\n'; break;
                case 'r': *dest = '\r'; break;
                case 't': *dest = '\t'; break;
                case 'v': *dest = '\v'; break;
                case 0x22: *dest = 0x22; break;
                case 0x5c: *dest = 0x5c; break;
                case 0x00:
                    error("unexpected end of string \x22%s\x22", yytext);
                    break;
                default:
                    error("unexpected escape sequence \x22\x5c%c\x22", *src);
                    break;
                }
            } else {
                *dest = *src;
            }
        }
        *dest = '\0';
    }
    Node *node = list_find(string_list, value, strlen(value) + sizeof(char));
    if (node == NULL) {
        node = new_node(VALUE_NODE, STRING, value);
        list_append(string_list, node);
    }
    return node;
}

Node *number(const char *yytext) {
    assert(strlen(yytext));
    NumberValue *value = malloc(sizeof(NumberValue));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value);
}
