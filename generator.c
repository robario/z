#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./parser.h"
#include "./generator.h"

extern FILE *yyout;

static const size_t SPSIZE = 64 / 8;

#if 1 <= DEBUG
#include <stdarg.h>

static void debug_indent(int level) {
    static int indent = 0;
    if (level < 0) {
        indent += level;
    }
    fputc('#', stderr);
    for (int i = 0; i < indent; ++i) {
        fputc('\t', stderr);
    }
    if (0 < level) {
        indent += level;
    }
}

#define debug(level, ...) (debug_indent(level), fprintf(stderr, __VA_ARGS__), fputc('\n', stderr))

static void mnemonic_printf(const char *format, ...) {
    static ssize_t sp = 0;

    va_list ap;
    va_start(ap, format);
    char *buffer = malloc(vsnprintf(NULL, 0, format, ap) + 1);
    va_end(ap);
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    fputs(buffer, stderr);

    ssize_t diff = 0;
    if (sp != 0 && strcmp(buffer, "ret") == 0) {
        fputs(" ; unbalanced stack\n", stderr);
        exit(1);
    } else if (strncmp(buffer, "push ", 5) == 0) {
        diff = -SPSIZE;
    } else if (strncmp(buffer, "pop ", 4) == 0) {
        diff = SPSIZE;
    } else if (sscanf(buffer + 4, "rsp, %ld", &diff) != 0) {
        if (strncmp(buffer, "sub ", 4) == 0) {
            diff = -diff;
        } else if (strncmp(buffer, "add ", 4) == 0) {
            diff = +diff;
        }
    }

    if (diff != 0) {
        sp += diff;
        fprintf(stderr, " \t; {%ld}", sp / -SPSIZE);
    }

    fputs(buffer, yyout);
}
#else
#define debug_indent(...) ((void)0)
#define debug(...) ((void)0)
#define mnemonic_printf(...) fprintf(yyout, __VA_ARGS__)
#endif

#define mnemonic_begin(...) (debug_indent(0), fputc('\t', yyout), mnemonic_printf(__VA_ARGS__))
#define mnemonic_end() mnemonic_printf("\n")
#define mnemonic(...) (mnemonic_begin(__VA_ARGS__), mnemonic_end())
#define label(...) (debug_indent(0), mnemonic_printf(__VA_ARGS__), mnemonic_printf("\n"))

ENUM_DEFINE(NodeClass);

void generate(Node *node) {
    static const char *const registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    static ProgramValue *program;
    static FunctionValue *declaration;

    if (node == NULL) {
        return;
    }
    debug(+1, "<%s, %s> {", enum_NodeClass(node->class), enum_yytokentype(node->type));

    switch (node->class) {
    case VALUE_NODE:
        switch (node->type) {
        case NUMBER:
            mnemonic("mov rax, %lld", *(long long int *)node->value);
            break;
        case STRING: {
            size_t index = list_find_cell(program->string_list, node, false)->index;
            mnemonic("lea rax, [rip + string.%zu]", index);
            break;
        }
        case LOCATOR: {
            size_t index = list_find_cell(declaration->locator_table, node, false)->index;
            mnemonic("mov rax, rbp");
            mnemonic("sub rax, %zu", SPSIZE * (index + 1));
            break;
        }
        case DELOCATOR:
            generate(node->value);
            mnemonic("pop rax");
            mnemonic("mov rax, [rax]");
            break;
        case FUNCTION: {
            size_t index = list_find_cell(program->function_list, node, false)->index;
            mnemonic("lea rax, [rip + function.%zu]", index);
            break;
        }
        case CALL: {
            FunctionValue *call = (FunctionValue *)node->value;
            size_t total_stacks = list_size(call->parameter_list);
            size_t actual_stacks = 6 < total_stacks ? total_stacks - 6 : 0;

            // 16 bytes stack alignment
            mnemonic("mov r11, rsp");
            mnemonic("add r11, %zu", SPSIZE * (actual_stacks + 1));
            mnemonic("and r11, 0x000000000000000F");
            mnemonic("sub rsp, r11");
            mnemonic("push r11");

            // parameters
            if (call->parameter_list != NULL) {
                generate(call->parameter_list);
                mnemonic("add rsp, %zu", SPSIZE * total_stacks);
                for (size_t i = 0; i < total_stacks; ++i) {
                    mnemonic("sub rsp, %zu", SPSIZE * 1);
                    if (i < 6) {
                        mnemonic("pop %s", registers[i]);
                    } else {
                        mnemonic("pop rax");
                        mnemonic("mov [rsp + %zu], rax", SPSIZE * (6 + (i - 6) - (actual_stacks - (i - 6))));
                    }
                    mnemonic("sub rsp, %zu", SPSIZE * 1);
                }
                mnemonic("add rsp, %zu", SPSIZE * (total_stacks - actual_stacks));
            }

            // call
            if (call->body->type == IDENTIFIER) {
                mnemonic("mov rax, 0");
                mnemonic("call _%s", call->body->value);
            } else {
                generate(call->body);
                mnemonic("pop r11");
                mnemonic("mov rax, 0");
                mnemonic("call r11");
            }

            // cleanup
            if (call->parameter_list != NULL) {
                mnemonic("add rsp, %zu", SPSIZE * actual_stacks);
            }
            mnemonic("pop r11");
            mnemonic("add rsp, r11");
            break;
        }
        default:
            break;
        }
        mnemonic("push rax");
        break;
    case OPERATOR_NODE:
        generate(node->left);
        if (node->right != NULL) {
            generate(node->right);
            mnemonic("pop rdi");
        }
        mnemonic("pop rax");
        switch (node->type) {
        case UMINUS:
            mnemonic("neg rax");
            break;
        case MULTIPLY:
            mnemonic("mul rdi");
            break;
        case DIVIDE:
            mnemonic("mov rdx, 0");
            mnemonic("div rdi");
            break;
        case ADD:
            mnemonic("add rax, rdi");
            break;
        case SUBTRACT:
            mnemonic("sub rax, rdi");
            break;
        case ASSIGN:
            mnemonic("mov [rax], rdi");
            mnemonic("mov rax, [rax]");
            break;
        default:
            break;
        }
        mnemonic("push rax");
        break;
    case ACCUMULABLE_LIST_NODE:
        for (Node *cell = node; cell != NULL; cell = cell->right) {
            generate(cell->node);
        }
        break;
    case SEQUENTIAL_LIST_NODE:
        for (Node *cell = node; cell != NULL; cell = cell->right) {
            generate(cell->node);
            mnemonic("pop rax");
        }
        mnemonic("push rax");
        break;
    case GENERAL_NODE:
        switch (node->type) {
        case PROGRAM:
            program = NULL;
            declaration = NULL;

            program = node->value;
            mnemonic(".intel_syntax noprefix");
            mnemonic(".text");
            mnemonic(".global _start");
            label("_start:");
            generate(program->body);
            mnemonic("pop rax");
            mnemonic("jmp rax");

            for (Node *cell = program->function_list; cell != NULL; cell = cell->right) {
                label("function.%zu:", cell->index);

                declaration = cell->node->value;
                mnemonic("push rbp");
                mnemonic("mov rbp, rsp");
                mnemonic("sub rsp, %zu", SPSIZE * list_size(declaration->locator_table));

                for (Node *formal_cell = declaration->parameter_list; formal_cell != NULL; formal_cell = formal_cell->right) {
                    generate(formal_cell->node);
                    mnemonic("pop rax");
                    size_t index = formal_cell->index;
                    if (index < 6) {
                        mnemonic("mov [rax], %s", registers[index]);
                    } else {
                        mnemonic("mov r11, [rbp + %zu]", SPSIZE * (1 + (index - 6) + 1));
                        mnemonic("mov [rax], r11");
                    }
                }

                if (declaration->body != NULL) {
                    generate(declaration->body);
                    mnemonic("pop rax");
                }
                mnemonic("add rsp, %zu", SPSIZE * list_size(declaration->locator_table));
                mnemonic("mov rsp, rbp");
                mnemonic("pop rbp");
                mnemonic("ret");
            }

            mnemonic(".data");
            for (Node *cell = program->string_list; cell != NULL; cell = cell->right) {
                label("string.%zu:", cell->index);
                mnemonic_begin(".byte");
                for (const char *c = cell->node->value; *c != '\0'; ++c) {
                    mnemonic_printf(" 0x%02X,", *c);
                }
                mnemonic_printf(" 0x00");
                mnemonic_end();
            }
            break;
        default:
            break;
        }
        break;
    case INCOMPLETE_NODE:
        break;
    }

    debug(-1, "}");
}
