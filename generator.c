#include <stdio.h>
#include "./generator.h"

extern FILE *yyout;

static const ssize_t SPSIZE = 64 / 8;

#if DEBUG
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static void debug_printf(const char *format, ...) {
    static size_t indent = 0;
    int level = 0;
    char *p = strpbrk(format, "{}");
    if (p != NULL) {
        level = -(*p - 0x7c);
    }
    if (0 < level) {
        indent += level;
    }
    fputc('#', stderr);
    for (size_t i = 1; i < indent; ++i) {
        fputc('\t', stderr);
    }
    if (level < 0) {
        indent += level;
    }

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

static void mnemonic_printf(const char *format, ...) {
    static ssize_t sp = 0;

    char *buffer;
    va_list ap;
    va_start(ap, format);
    vasprintf(&buffer, format, ap);
    va_end(ap);

    fputs(buffer, stderr);

    ssize_t diff = 0;
    if (sp != 0 && strcmp(buffer, "ret") == 0) {
        error(" ; unbalanced stack");
    } else if (strncmp(buffer, "push ", 5) == 0) {
        diff = -SPSIZE;
    } else if (strncmp(buffer, "pop ", 4) == 0) {
        diff = +SPSIZE;
    } else if (sscanf(buffer + 4, "rsp, %ld", &diff) != 0) {
        if (strncmp(buffer, "sub ", 4) == 0) {
            diff = -diff;
        } else if (strncmp(buffer, "add ", 4) == 0) {
            diff = +diff;
        }
    }
    if (diff != 0) {
        sp += diff;
        fprintf(stderr, " \t; {%ld}", -(sp / SPSIZE));
    }

    fputs(buffer, yyout);
}
#else
#define debug_printf(...) ((void)0)
#define mnemonic_printf(...) fprintf(yyout, __VA_ARGS__)
#endif

#define mnemonic_indent() mnemonic_printf("\t")
#define mnemonic_begin()   \
    do {                   \
        debug_printf("");  \
        mnemonic_indent(); \
    } while (0)
#define mnemonic_end() mnemonic_printf("\n")

#define mnemonic(...)                 \
    do {                              \
        mnemonic_begin();             \
        mnemonic_printf(__VA_ARGS__); \
        mnemonic_end();               \
    } while (0)

#define label(...)                    \
    do {                              \
        debug_printf("");             \
        mnemonic_printf(__VA_ARGS__); \
        mnemonic_end();               \
    } while (0)

#define debug(format, ...) debug_printf(format "\n", ##__VA_ARGS__)

void generate(Node *node) {
    static Node *program;
    static Node *function;

    debug("%s%s {", node_class_string(node), node_string(node));

    switch (node->class) {
    case VALUE_NODE:
        switch (node->type) {
        case NUMBER:
            mnemonic("mov rax, %lld", NumberValue(node));
            break;
        case DELOCATOR:
            generate(NodeValue(node));
            mnemonic("pop rax");
            mnemonic("mov rax, [rax]");
            break;
        case LOCATOR:
            mnemonic("mov rax, rbp");
            mnemonic("sub rax, %zu", SPSIZE * (list_index(FunctionValue(function)->table, node) + 1));
            break;
        case FUNCTION:
            mnemonic("lea rax, [rip + function.%zu]", list_index(ProgramValue(program)->function_list, node));
            break;
        case CALL:
            // 16 bytes stack alignment
            mnemonic("mov rcx, rsp");
            mnemonic("add rcx, %zu", SPSIZE * 1);
            mnemonic("and rcx, 0x000000000000000F");
            mnemonic("sub rsp, rcx");
            mnemonic("push rcx");

            // call
            generate(FunctionValue(node)->body);
            mnemonic("pop rax");
            mnemonic("call rax");

            // cleanup
            mnemonic("pop rcx");
            mnemonic("add rsp, rcx");
            break;
        default:
            assert(0);
            break;
        }
        mnemonic("push rax");
        break;
    case OPERATOR_NODE:
        generate(OperatorValue(node)->lhs);
        if (OperatorValue(node)->rhs != NULL) {
            generate(OperatorValue(node)->rhs);
            mnemonic("pop rcx");
        }
        mnemonic("pop rax");
        switch (node->type) {
        case MINUS:
            mnemonic("neg rax");
            break;
        case MULTIPLY:
            mnemonic("imul rcx");
            break;
        case DIVIDE:
            mnemonic("mov rdx, 0");
            mnemonic("idiv rcx");
            break;
        case ADD:
            mnemonic("add rax, rcx");
            break;
        case SUBTRACT:
            mnemonic("sub rax, rcx");
            break;
        case ASSIGN:
            mnemonic("mov [rax], rcx");
            mnemonic("mov rax, [rax]");
            break;
        default:
            assert(0);
            break;
        }
        mnemonic("push rax");
        break;
    case LIST_NODE:
        switch (node->type) {
        case SEQUENTIAL:
            for (size_t index = 0; index < ListValue(node)->size; ++index) {
                generate(ListValue(node)->nodes[index]);
                mnemonic("pop rax");
            }
            mnemonic("push rax");
            break;
        default:
            assert(0);
            break;
        }
        break;
    case GENERAL_NODE:
        switch (node->type) {
        case PROGRAM:
            program = node;
            mnemonic(".intel_syntax noprefix");
            mnemonic(".text");
            mnemonic(".global _start");
            label("_start:");
            generate(ProgramValue(program)->body);
            mnemonic("pop rax");
            mnemonic("jmp rax");

            for (size_t index = 0; index < ListValue(ProgramValue(program)->function_list)->size; ++index) {
                label("function.%zu:", index);
                function = ListValue(ProgramValue(program)->function_list)->nodes[index];
                size_t total_stacks = ListValue(FunctionValue(function)->table)->size;

                mnemonic("push rbp");
                mnemonic("mov rbp, rsp");
                mnemonic("sub rsp, %zu", SPSIZE * total_stacks);

                generate(FunctionValue(function)->body);
                mnemonic("pop rax");

                mnemonic("add rsp, %zu", SPSIZE * total_stacks);
                mnemonic("mov rsp, rbp");
                mnemonic("pop rbp");
                mnemonic("ret");
            }
            break;
        default:
            assert(0);
            break;
        }
        break;
    }

    debug("}");
}
