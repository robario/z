#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"

int yyparse(Node **ast);
void yyprint(FILE *yyoutput, int yytype, const YYSTYPE yyvalue);

static Node *locator_table_list;
static Node *locator_table;
static ProgramValue *program_value;

int parse(Node** ast) {
    program_value = malloc(sizeof(ProgramValue));
    program_value->function_list = NULL;
    program_value->string_list = NULL;
    program_value->body = NULL;
    locator_table_list = NULL;
    locator_table = NULL;
    return yyparse(ast);
}

static Node *new_node(NodeClass class, NodeType type, void *value, Node *left, Node *right) {
    Node *self = malloc(sizeof(Node));
    self->class = class;
    self->type = type;
    self->value = value;
    self->left = left;
    self->right = right;
    return self;
}

static Node *new_cell(NodeClass class, size_t index, Node *node, Node *left, Node *right) {
    Node *self = malloc(sizeof(Node));
    self->class = class;
    self->index = index;
    self->node = node;
    self->left = left;
    self->right = right;
    return self;
}

size_t list_size(Node *list_head) {
    return list_head == NULL ? 0 : list_head->left->index + 1;
}

#if 2 <= DEBUG
static void list_print(Node *list_head) {
    fputc('(', stderr);
    if (list_head != NULL) {
        yyprint(stderr, list_head->type, list_head);
    } else {
        fprintf(stderr, "0x%08llx", (unsigned long long int)NULL);
    }
    fputc(')', stderr);
    fputc('[', stderr);
    for (Node *cell = list_head; cell != NULL; cell = cell->right) {
        if (cell != list_head) {
            fputc(',', stderr);
        }
        yyprint(stderr, cell->node->type, cell->node);
    }
    fputc(']', stderr);
}
#endif

Node *list_find_cell(Node *list_head, Node *node, bool equivalence) {
    if (node == NULL) {
        return NULL;
    }
#if 2 <= DEBUG
    fputs("## ", stderr);
    yyprint(stderr, node->type, node);
    fputs(" from ", stderr);
    list_print(list_head);
#endif
    for (Node *cell = list_head; cell != NULL; cell = cell->right) {
        if (cell->node == node ||
            (equivalence && strcmp(cell->node->value, node->value) == 0)) {
#if 2 <= DEBUG
            fputs(" found\n", stderr);
#endif
            return cell;
        }
    }
#if 2 <= DEBUG
    fputs(" not found\n", stderr);
#endif
    return NULL;
}

Node *list_append(Node **list_head, Node *node) {
    Node *cell = new_cell(INCOMPLETE_NODE, 0, node, NULL, NULL);
    if (*list_head == NULL) {
        *list_head = cell;
    } else {
        cell->class = (*list_head)->class;
        cell->index = (*list_head)->left->index + 1;
        (*list_head)->left->right = cell;
    }
    cell->left = (*list_head)->left;
    (*list_head)->left = cell;
    return node;
}

Node *new_accumulable_list(Node **list_head, Node *node) {
    *list_head = NULL;
    list_append(list_head, node);
    (*list_head)->class = ACCUMULABLE_LIST_NODE;
    return node;
}

Node *new_sequential_list(Node **list_head, Node *node) {
    *list_head = NULL;
    list_append(list_head, node);
    (*list_head)->class = SEQUENTIAL_LIST_NODE;
    return node;
}

Node *program(Node *body) {
    program_value->body = body;
    return new_node(GENERAL_NODE, PROGRAM, program_value, NULL, NULL);
}

void new_locator_table(void) {
#if 2 <= DEBUG
    fprintf(stderr, "## new_locator_table()\n");
#endif
    list_append(&locator_table_list, locator_table);
    locator_table = NULL;
}

void restore_locator_table(void) {
    Node *cell = list_find_cell(locator_table_list, locator_table, false);
    if (cell == NULL) {
        cell = locator_table_list;
    }
    if (cell == NULL) {
        locator_table = NULL;
    } else {
        locator_table = cell->left->node;
    }
#if 2 <= DEBUG
    fputs("## restore_locator_table() ", stderr);
    list_print(locator_table);
    fputc('\n', stderr);
#endif
}

Node *function(Node *formal, Node *body) {
    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->locator_table = locator_table;
    value->parameter_list = formal;
    value->body = body;
    restore_locator_table();

    Node *function = new_node(VALUE_NODE, FUNCTION, value, NULL, NULL);
    return list_append(&program_value->function_list, function);
}

Node *binary(NodeType type, Node *left, Node *right) {
    return new_node(OPERATOR_NODE, type, NULL, left, right);
}

Node *unary(NodeType type, Node *operand) {
    return binary(type, operand, NULL);
}

Node *call(Node *delocator, Node *actual) {
    FunctionValue *value = malloc(sizeof(FunctionValue));
    value->locator_table = NULL;
    value->parameter_list = actual;
    value->body = delocator;
    return new_node(VALUE_NODE, CALL, value, NULL, NULL);
}

Node *delocator(Node *identifier) {
    Node *cell = list_find_cell(locator_table, identifier, true);
    if (cell == NULL) {
        fprintf(stderr, "use of undeclared identifier '%s'\n", identifier->value);
        return identifier; // XXX: for external call
        // exit(1);
    }
    return new_node(VALUE_NODE, DELOCATOR, cell->node, NULL, NULL);
}

Node *locator(Node *identifier) {
    Node *cell = list_find_cell(locator_table, identifier, true);
    if (cell == NULL) {
#if 2 <= DEBUG
        fputs("## ", stderr);
        yyprint(stderr, identifier->type, identifier);
        fputs(" into ", stderr);
        list_print(locator_table);
        fputc('\n', stderr);
#endif
        identifier->type = LOCATOR;
        list_append(&locator_table, identifier);

        return identifier;
    }
    return cell->node;
}

Node *identifier(const char *yytext) {
    char *value = strdup(yytext);
    return new_node(VALUE_NODE, IDENTIFIER, value, NULL, NULL);
}

Node *string(const char *yytext, const char delimiter) {
    char *const value = strdup(yytext);
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
                    fprintf(stderr, "unexpected end of string \x22%s\x22\n", yytext);
                    exit(1);
                    break;
                default:
                    fprintf(stderr, "unexpected escape sequence \x22\x5c%c\x22\n", *src);
                    exit(1);
                    break;
                }
            } else {
                *dest = *src;
            }
        }
        *dest = '\0';
    }
    Node *node = new_node(VALUE_NODE, STRING, value, NULL, NULL);

    Node *cell = list_find_cell(program_value->string_list, node, true);
    if (cell == NULL) {
#if 2 <= DEBUG
        fputs("## ", stderr);
        yyprint(stderr, node->type, node);
        fputs(" into ", stderr);
        list_print(program_value->string_list);
        fputc('\n', stderr);
#endif
        list_append(&program_value->string_list, node);

        return node;
    }
    return cell->node;
}

Node *number(const char *yytext) {
    long long int *value = malloc(sizeof(long long int));
    *value = strtoll(yytext, NULL, 10);
    return new_node(VALUE_NODE, NUMBER, value, NULL, NULL);
}
