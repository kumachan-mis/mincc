#include "gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "memory.h"


void put_code(FILE* file_ptr, Vector* codes);

// expression-code-generator
void gen_primary_expr_code(Ast* ast, CodeEnvironment* env);
void gen_postfix_expr_code(Ast* ast, CodeEnvironment* env);
void gen_arg_expr_list_code(Ast* ast, CodeEnvironment* env);
void gen_unary_expr_code(Ast* ast, CodeEnvironment* env);
void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env);
void gen_shift_expr_code(Ast* ast, CodeEnvironment* env);
void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env);
void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env);
void gen_logical_expr_code(Ast* ast, CodeEnvironment* env);
void gen_assignment_expr_code(Ast* ast, CodeEnvironment* env);
void gen_expr_code(Ast* ast, CodeEnvironment* env);

// statement-code-generator
void gen_jump_stmt_code(Ast* ast, CodeEnvironment* env);
void gen_expr_stmt_code(Ast* ast, CodeEnvironment* env);
void gen_stmt_code(Ast* ast, CodeEnvironment* env);

// code-environment
CodeEnvironment* code_environment_new();
void append_code(Vector* codes, char* format, ...);
int get_stack_offset(CodeEnvironment* env, char* value_ident);
int create_stack_offset(CodeEnvironment* env, char* value_ident);
int get_or_create_stack_offset(CodeEnvironment* env, char* value_ident);
void code_environment_delete(CodeEnvironment* env);

// assertion
void assert_code_gen(int condition);


void print_code(AstList* astlist) {
    if (astlist_top(astlist) == NULL) return;

    CodeEnvironment* env = code_environment_new();
    size_t i = 0, size = astlist->inner_vector->size;
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;
        gen_stmt_code(ast, env);
        astlist_pop(astlist);
    }

    Vector* header_codes = vector_new();
    append_code(header_codes, "\t.global _main\n");
    append_code(header_codes, "_main:\n");
    append_code(header_codes, "\tpush %%rbp\n");
    append_code(header_codes, "\tmov %%rsp, %%rbp\n");
    append_code(header_codes, "\tsub $%d, %%rsp\n", env->stack_offset);

    Vector* footer_codes = vector_new();
    append_code(footer_codes, ".L_main_return:\n");
    append_code(footer_codes, "\tmov %%rbp, %%rsp\n");
    append_code(footer_codes, "\tpop %%rbp\n");
    append_code(footer_codes, "\tret\n");

    put_code(stdout, header_codes);
    put_code(stdout, env->codes);
    put_code(stdout, footer_codes);

    vector_delete(footer_codes);
    vector_delete(header_codes);
    code_environment_delete(env);
}

void put_code(FILE* file_ptr, Vector* codes) {
    size_t i = 0, size = codes->size;
    for (i = 0; i < size; i++) {
        char* str = (char*)vector_at(codes, i);
        fputs(str, file_ptr);
    }
}

// expression-code-generator
void gen_primary_expr_code(Ast* ast, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_INT:
            append_code(env->codes, "\tpush $%d\n", ast->value_int);
            break;
        case AST_VAR: {
            int stack_offset = get_stack_offset(env, ast->value_ident);
            append_code(env->codes, "\tmov -%d(%%rbp), %%eax\n", stack_offset);
            append_code(env->codes, "\tpush %%rax\n");
        }
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_postfix_expr_code(Ast* ast, CodeEnvironment* env) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);

    switch (ast->type) {
        case AST_CALL:
            assert_code_gen(lhs->type == AST_VAR);
            gen_arg_expr_list_code(rhs, env);
            append_code(env->codes, "\tcall _%s\n", lhs->value_ident);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_arg_expr_list_code(Ast* ast, CodeEnvironment* env) {
    size_t num_args = ast->children->size;
    assert_code_gen(num_args <= 6);

    char* arg_register[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };
    size_t i = 0;
    for (i = 0; i < num_args; i++) {
        gen_expr_code(ast_nth_child(ast, i), env);
        append_code(env->codes, "\tpop %%%s\n", arg_register[i]);
    }
}

void gen_unary_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), env);

    switch (ast->type) {
        case AST_POSI:
            /* Do Nothing */
            break;
        case AST_NEGA:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tneg %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_NOT:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tnot %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_LNOT:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%eax\n");
            append_code(env->codes, "\tsete %%al\n");
            append_code(env->codes, "\tmovzb %%al, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), env);
    gen_expr_code(ast_nth_child(ast, 1), env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_ADD:
            append_code(env->codes, "\tadd %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_SUB:
            append_code(env->codes, "\tsub %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_MUL:
            append_code(env->codes, "\timul %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_DIV:
            append_code(env->codes, "\tcltd\n");
            append_code(env->codes, "\tidiv %%edi\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_MOD:
            append_code(env->codes, "\tcltd\n");
            append_code(env->codes, "\tidiv %%edi\n");
            append_code(env->codes, "\tpush %%rdx\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_shift_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), env);
    gen_expr_code(ast_nth_child(ast, 1), env);

    append_code(env->codes, "\tpop %%rcx\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_LSHIFT:
            append_code(env->codes, "\tsal %%cl, %%eax\n");
            break;
        case AST_RSHIFT:
            append_code(env->codes, "\tsar %%cl, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), env);
    gen_expr_code(ast_nth_child(ast, 1), env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp %%edi, %%eax\n");
    switch (ast->type) {
        case AST_EQ:
            append_code(env->codes, "\tsete %%al\n");
            break;
        case AST_NEQ:
            append_code(env->codes, "\tsetne %%al\n");
            break;
        case AST_LT:
            append_code(env->codes, "\tsetl %%al\n");
            break;
        case AST_GT:
            append_code(env->codes, "\tsetg %%al\n");
            break;
        case AST_LEQ:
            append_code(env->codes, "\tsetle %%al\n");
            break;
        case AST_GEQ:
            append_code(env->codes, "\tsetge %%al\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), env);
    gen_expr_code(ast_nth_child(ast, 1), env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_OR:
            append_code(env->codes, "\tor %%edi, %%eax\n");
            break;
        case AST_XOR:
            append_code(env->codes, "\txor %%edi, %%eax\n");
            break;
        case AST_AND:
            append_code(env->codes, "\tand %%edi, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_logical_expr_code(Ast* ast, CodeEnvironment* env) {
    int exit_labno = env->num_labels; env->num_labels++;

    gen_expr_code(ast_nth_child(ast, 0), env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    switch (ast->type) {
        case AST_LOR:
            append_code(env->codes, "\tjne .L%d\n", exit_labno);
            break;
        case AST_LAND:
            append_code(env->codes, "\tje .L%d\n", exit_labno);
            break;
        default:
            assert_code_gen(0);
    }

    gen_expr_code(ast_nth_child(ast, 1), env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    append_code(env->codes, ".L%d:\n", exit_labno);
    append_code(env->codes, "\tsetne %%al\n");
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_assignment_expr_code(Ast* ast, CodeEnvironment* env) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);

    gen_expr_code(rhs, env);
    append_code(env->codes, "\tpop %%rax\n");
    assert_code_gen(lhs->type == AST_VAR);

    switch (ast->type) {
        case AST_ASSIGN:
            break;
        default:
            assert_code_gen(0);
    }

    int stack_offset = get_or_create_stack_offset(env, lhs->value_ident);
    append_code(env->codes, "\tmov %%eax, -%d(%%rbp)\n", stack_offset);
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_expr_code(Ast* ast, CodeEnvironment* env) {
     AstType type = ast->type;
    
    if (is_primary_expr(type)) {
        gen_primary_expr_code(ast, env);
    } else if (is_postfix_expr(type)) {
        gen_postfix_expr_code(ast, env);
    } else if (is_unary_expr(type)) {
        gen_unary_expr_code(ast, env);
    } else if (
        is_multiplicative_expr(type) ||
        is_additive_expr(type)
    ) {
        gen_arithmetical_expr_code(ast, env);
    } else if (is_shift_expr(type)) {
        gen_shift_expr_code(ast, env);
    } else if (
        is_relational_expr(type) ||
        is_equality_expr(type)
    ) {
        gen_comparative_expr_code(ast, env);
    } else if (is_bitwise_expr(type)) {
        gen_bitwise_expr_code(ast, env);
    } else if (is_logical_expr(type)) {
        gen_logical_expr_code(ast, env);
    } else if (is_assignment_expr(type)) {
        gen_assignment_expr_code(ast, env);
    } else {
        assert_code_gen(0);
    }
}

// statement-code-generator
void gen_jump_stmt_code(Ast* ast, CodeEnvironment* env) {
    Ast* expr = ast_nth_child(ast, 0);

    switch (ast->type) {
        case AST_RETURN_STMT:
            gen_expr_code(expr, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tjmp .L_main_return\n");
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_expr_stmt_code(Ast* ast, CodeEnvironment* env) {
    Ast* expr = ast_nth_child(ast, 0);

    switch (ast->type) {
        case AST_EXPR_STMT:
            gen_expr_code(expr, env);
            append_code(env->codes, "\tadd $8, %%rsp\n");
            break;
        case AST_NULL_STMT:
            append_code(env->codes, "\tnop\n");
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_stmt_code(Ast* ast, CodeEnvironment* env) {
    AstType type = ast->type;

    if (is_expr_stmt(type)) {
        gen_expr_stmt_code(ast, env);
    } else if (is_jump_stmt(type)) {
        gen_jump_stmt_code(ast, env);
    } else  {
        assert_code_gen(0);
    }
}

// code-environment
CodeEnvironment* code_environment_new() {
    CodeEnvironment* env = (CodeEnvironment*)safe_malloc(sizeof(CodeEnvironment));
    env->num_labels = 0;
    env->stack_offset = 0;
    env->var_map = map_new();
    env->codes = vector_new();
    return env;
}

void append_code(Vector* codes, char* format, ...) {
    char buffer[511];
    va_list list;
    va_start(list, format);
    int success = vsnprintf(buffer, 510, format, list);
    va_end(list);
    assert_code_gen(success);
    vector_push_back(codes, str_new(buffer));
}

int get_stack_offset(CodeEnvironment* env, char* value_ident) {
    int* offset_ptr = (int*)map_find(env->var_map, value_ident);
    assert_code_gen(offset_ptr != NULL);
    return *offset_ptr;
}

int create_stack_offset(CodeEnvironment* env, char* value_ident) {
    env->stack_offset += 8;
    int* offset_ptr = int_new(env->stack_offset);
    map_insert(env->var_map, value_ident, offset_ptr);
    return *offset_ptr;
}

int get_or_create_stack_offset(CodeEnvironment* env, char* value_ident) {
    int* offset_ptr = (int*)map_find(env->var_map, value_ident);
    if (offset_ptr != NULL) return *offset_ptr;
    return create_stack_offset(env, value_ident);
}

void code_environment_delete(CodeEnvironment* env) {
    map_delete(env->var_map);
    vector_delete(env->codes);
    free(env);
}

// assertion
void assert_code_gen(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to generate code\n");
    exit(1);
}
