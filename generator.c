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
    static const char *const registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    static const size_t total_registers = sizeof(registers) / sizeof(char *);
    static Node *global_list;
    static Node *program;
    static Node *function;

    debug("%s%s {", node_class_string(node), node_string(node));

    switch (node->class) {
    case VALUE_NODE:
        switch (node->type) {
            size_t total_stacks;
            size_t actual_stacks;
        case NUMBER:
            mnemonic("mov rax, %lld", NumberValue(node));
            break;
        case STRING:
            mnemonic("lea rax, [rip + string.%zu]", list_index(ProgramValue(program)->string_list, node));
            break;
        case DELOCATOR:
            generate(NodeValue(node));
            mnemonic("pop rax");
            if (!is_extern(NodeValue(node)))
//            if (!is_global(NodeValue(node)))
            {
                mnemonic("mov rax, [rax]");
            }
            break;
        case LOCATOR:
            if (is_global(node)) {
                mnemonic("mov rax, [rip + _%s@GOTPCREL]", strrchr(StringValue(node), '.') + sizeof(char));
            } else {
                mnemonic("mov rax, rbp");
                mnemonic("sub rax, %zu", SPSIZE * (list_index(FunctionValue(function)->table, node) + 1));
            }
            break;
        case FUNCTION:
            mnemonic("lea rax, [rip + function.%zu]", list_index(ProgramValue(program)->function_list, node));
            break;
        case CALL:
            total_stacks = ListValue(FunctionValue(node)->parameter_list)->size;
            actual_stacks = total_registers < total_stacks ? total_stacks - total_registers : 0;

            // 16 bytes stack alignment
            mnemonic("mov rcx, rsp");
            mnemonic("add rcx, %zu", SPSIZE * (actual_stacks + 1));
            mnemonic("and rcx, 0x000000000000000F");
            mnemonic("sub rsp, rcx");
            mnemonic("push rcx");

            // parameters
            generate(FunctionValue(node)->parameter_list);
            for (size_t dest = 0; dest < total_stacks; ++dest) {
                size_t src = total_stacks - dest - 1;
                if (dest < total_registers) {
                    mnemonic("mov %s, [rsp + %zu]", registers[dest], SPSIZE * src);
                } else {
                    mnemonic("mov rax, [rsp + %zu]", SPSIZE * src);
                    mnemonic("mov [rsp + %zu], rax", SPSIZE * dest);
                }
            }
            mnemonic("add rsp, %zu", SPSIZE * (total_stacks - actual_stacks));

            // call
            generate(FunctionValue(node)->body);
            mnemonic("pop rax");
            mnemonic("call rax");

            // cleanup
            mnemonic("add rsp, %zu", SPSIZE * actual_stacks);
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
            if (is_global(OperatorValue(node)->lhs) && !is_extern(OperatorValue(node)->lhs)) {
                const char *name = StringValue(OperatorValue(node)->lhs);

                Node *global = list_find(global_list, name, strlen(name) + sizeof(char));
                if (global != NULL) {
                    error("cannot assign global multiple: %s", node_string(OperatorValue(node)->lhs));
                }
                void *value = malloc(strlen(name) + sizeof(char) + sizeof(Node *));
                strcpy(value, name);
                global = new_node(GENERAL_NODE, OperatorValue(node)->rhs->type, value);
                list_append(global_list, global);
                *(Node **)(global->value + strlen(name) + sizeof(char)) = OperatorValue(node)->rhs;
            }
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
        case ACCUMULABLE:
            for (size_t index = 0; index < ListValue(node)->size; ++index) {
                generate(ListValue(node)->nodes[index]);
            }
            break;
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
            global_list = list_new();

            mnemonic(".intel_syntax noprefix");
            mnemonic(".text");
            mnemonic(".global _start");
            label("_start:");
            generate(ProgramValue(program)->body);
            mnemonic("pop rax");
            mnemonic("jmp rax");

            for (size_t index = 0; index < ListValue(ProgramValue(program)->function_list)->size; ++index) {
                function = ListValue(ProgramValue(program)->function_list)->nodes[index];
                label("function.%zu:", index);
                size_t total_stacks = ListValue(FunctionValue(function)->table)->size;

                mnemonic("push rbp");
                mnemonic("mov rbp, rsp");
                mnemonic("sub rsp, %zu", SPSIZE * total_stacks);

                for (size_t index = 0; index < ListValue(FunctionValue(function)->parameter_list)->size; ++index) {
                    generate(ListValue(FunctionValue(function)->parameter_list)->nodes[index]);
                    mnemonic("pop rax");
                    if (index < total_registers) {
                        mnemonic("mov [rax], %s", registers[index]);
                    } else {
                        mnemonic("mov rcx, [rbp + %zu]", SPSIZE * (index - total_registers + 2)); // 2 = skip rbp and rip
                        mnemonic("mov [rax], rcx");
                    }
                }

                generate(FunctionValue(function)->body);
                mnemonic("pop rax");

                mnemonic("add rsp, %zu", SPSIZE * total_stacks);
                mnemonic("mov rsp, rbp");
                mnemonic("pop rbp");
                mnemonic("ret");
            }

            mnemonic(".data");
            for (size_t index = 0; index < ListValue(ProgramValue(program)->string_list)->size; ++index) {
                label("string.%zu:", index);
                mnemonic_begin();
                mnemonic_printf(".byte");
                for (const char *c = StringValue(ListValue(ProgramValue(program)->string_list)->nodes[index]); *c != '\0'; ++c) {
                    mnemonic_printf(" 0x%02X,", *c);
                }
                mnemonic_printf(" 0x00");
                mnemonic_end();
            }
            for (size_t index = 0; index < ListValue(global_list)->size; ++index) {
                Node *node = ListValue(global_list)->nodes[index];
                const char *name = node->value;
                mnemonic(".global _%s", name + sizeof(char));
                label("_%s:", name + sizeof(char));
                Node *n = *(Node **)(node->value + strlen(name) + sizeof(char));
                debug("%s%s", node_class_string(n), node_string(n));
                switch (n->type) {
                case FUNCTION:
                    mnemonic(".quad function.%zu", list_index(ProgramValue(program)->function_list, n));
                    break;
                case STRING:
                    mnemonic(".quad string.%zu", list_index(ProgramValue(program)->string_list, n));
                    break;
                case NUMBER:
                    mnemonic(".quad %lld", NumberValue(n));
                    break;
                default:
                    debug("%s%s", node_class_string(n), node_string(n));
                    assert(0);
                    break;
                }
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
