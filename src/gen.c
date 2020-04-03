#include "gen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "symboltable.h"
#include "memory.h"


static char* arg_register[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };


void put_code(FILE* file_ptr, Vector* codes);

// expression-code-generator
void gen_primary_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_postfix_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_arg_expr_list_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_unary_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_arithmetical_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_shift_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_comparative_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_bitwise_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_logical_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_assignment_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_null_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);

// address/lvalue-code-generator
void gen_address_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_lvalue_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);

// statement-code-generator
void gen_compound_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_expr_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_selection_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_iteration_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_jump_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);

// declaration-code-generator
void gen_declaration_list_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);
void gen_declaration_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);

// external-definition-generator
void gen_function_definition_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env);

// code-environment
CodeEnvironment* code_environment_new();
void append_code(Vector* codes, char* format, ...);
char* create_new_label(CodeEnvironment* env);
void code_environment_delete(CodeEnvironment* env);

// assertion
void assert_code_gen(int condition);


void print_code(AstList* astlist) {
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;

        CodeEnvironment* env = code_environment_new();
        gen_function_definition_code(ast, NULL, env);
        put_code(stdout, env->codes);

        code_environment_delete(env);
        astlist_pop(astlist);
    }
}

void put_code(FILE* file_ptr, Vector* codes) {
    size_t i = 0, size = codes->size;
    for (i = 0; i < size; i++) {
        char* str = (char*)vector_at(codes, i);
        fputs(str, file_ptr);
    }
}

// expression-code-generator
void gen_primary_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    int stack_index = 0;

    switch (ast->type) {
        case AST_IMM_INT:
            append_code(env->codes, "\tpush $%d\n", ast->value_int);
            break;
        case AST_IDENT:
            stack_index = symbol_table_get_stack_index(symbol_table, ast->value_ident);
            append_code(env->codes, "\tmov -%d(%%rbp), %%rax\n", stack_index);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_postfix_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);

    switch (ast->type) {
        case AST_FUNC_CALL:
            assert_code_gen(lhs->type == AST_IDENT);
            gen_arg_expr_list_code(rhs, symbol_table, env);
            append_code(env->codes, "\tcall _%s\n", lhs->value_ident);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_arg_expr_list_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    size_t num_args = ast->children->size;
    assert_code_gen(num_args <= 6);

    size_t i = 0;
    for (i = 0; i < num_args; i++) {
        gen_expr_code(ast_nth_child(ast, i), symbol_table, env);
    }
    for (i = 0; i < num_args; i++) {
        append_code(env->codes, "\tpop %%%s\n", arg_register[num_args - i - 1]);
    }
}

void gen_unary_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* child = ast_nth_child(ast, 0);

    switch (ast->type) {
        case AST_ADDR:
            gen_address_code(child, symbol_table, env);
            break;
        case AST_DEREF:
            gen_expr_code(child, symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tmov (%%rax), %%rax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_POSI:
            gen_expr_code(child, symbol_table, env);
            break;
        case AST_NEGA:
            gen_expr_code(child, symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tneg %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_NOT:
            gen_expr_code(child, symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tnot %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_LNOT:
            gen_expr_code(child, symbol_table, env);
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

void gen_arithmetical_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);

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

void gen_shift_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);

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

void gen_comparative_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);

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

void gen_bitwise_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);

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

void gen_logical_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    char* exit_label = create_new_label(env);

    gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    switch (ast->type) {
        case AST_LOR:
            append_code(env->codes, "\tjne .L%s\n", exit_label);
            break;
        case AST_LAND:
            append_code(env->codes, "\tje .L%s\n", exit_label);
            break;
        default:
            assert_code_gen(0);
    }

    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    append_code(env->codes, ".L%s:\n", exit_label);
    append_code(env->codes, "\tsetne %%al\n");
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");

    free(exit_label);
}

void gen_assignment_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);
    gen_lvalue_code(ast_nth_child(ast, 0), symbol_table, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");

    switch (ast->type) {
        case AST_ASSIGN:
            break;
        default:
            assert_code_gen(0);
    }

    append_code(env->codes, "\tmov %%rax, (%%rdi)\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_null_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
     switch (ast->type) {
        case AST_NULL:
            /* Do Nothing */
            break;
        default:
            assert_code_gen(0);
     }
}

void gen_expr_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
     AstType type = ast->type;
    
    if (is_primary_expr(type)) {
        gen_primary_expr_code(ast, symbol_table, env);
    } else if (is_postfix_expr(type)) {
        gen_postfix_expr_code(ast, symbol_table, env);
    } else if (is_unary_expr(type)) {
        gen_unary_expr_code(ast, symbol_table, env);
    } else if (
        is_multiplicative_expr(type) ||
        is_additive_expr(type)
    ) {
        gen_arithmetical_expr_code(ast, symbol_table, env);
    } else if (is_shift_expr(type)) {
        gen_shift_expr_code(ast, symbol_table, env);
    } else if (
        is_relational_expr(type) ||
        is_equality_expr(type)
    ) {
        gen_comparative_expr_code(ast, symbol_table, env);
    } else if (is_bitwise_expr(type)) {
        gen_bitwise_expr_code(ast, symbol_table, env);
    } else if (is_logical_expr(type)) {
        gen_logical_expr_code(ast, symbol_table, env);
    } else if (is_assignment_expr(type)) {
        gen_assignment_expr_code(ast, symbol_table, env);
    } else if (is_null_expr(type)) {
        gen_null_expr_code(ast, symbol_table, env);
    } else {
        assert_code_gen(0);
    }
}


// address/lvalue-code-generator
void gen_address_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    int stack_index = 0;

    switch(ast->type) {
        case AST_IDENT:
            stack_index = symbol_table_get_stack_index(symbol_table, ast->value_ident);
            append_code(env->codes, "\tlea -%d(%%rbp), %%rax\n", stack_index);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_DEREF:
            gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_lvalue_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    return gen_address_code(ast, symbol_table, env);
}

// statement-code-generator
void gen_compound_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    size_t i = 0, size = ast->children->size;
    switch (ast->type) {
        case AST_COMP_STMT:
            for (i = 0; i < size; i++) {
                Ast* child = ast_nth_child(ast, i);
                if (is_declaration_list(child->type)) {
                    gen_declaration_list_code(child, ast->symbol_table, env);
                } else {
                    gen_stmt_code(child, ast->symbol_table, env);
                }
            }
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_expr_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* child = NULL;

    switch (ast->type) {
        case AST_EXPR_STMT:
            child = ast_nth_child(ast, 0);
            gen_expr_code(child, symbol_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tadd $8, %%rsp\n");
            }
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_selection_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_IF_STMT:
            gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            if (ast->children->size == 2) {
                char* exit_label = create_new_label(env);
                append_code(env->codes, "\tje .L%s\n", exit_label);
                gen_stmt_code(ast_nth_child(ast, 1), symbol_table, env);
                append_code(env->codes, ".L%s:\n",   exit_label);
                free(exit_label);
            } else {
                char* else_label = create_new_label(env);
                char* exit_label = create_new_label(env);
                append_code(env->codes, "\tje .L%s\n", else_label);
                gen_stmt_code(ast_nth_child(ast, 1), symbol_table, env);
                append_code(env->codes, "\tjmp .L%s\n", exit_label);
                append_code(env->codes, ".L%s:\n",   else_label);
                gen_stmt_code(ast_nth_child(ast, 2), symbol_table, env);
                append_code(env->codes, ".L%s:\n",   exit_label);
                free(else_label);
                free(exit_label);
            }
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_iteration_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* child = NULL;
    char* entry_label = create_new_label(env);
    char* exit_label = create_new_label(env);

    switch (ast->type) {
        case AST_WHILE_STMT: 
            append_code(env->codes, ".L%s:\n",   entry_label);
            gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            append_code(env->codes, "\tje .L%s\n", exit_label);
            gen_stmt_code(ast_nth_child(ast, 1), symbol_table, env);
            append_code(env->codes, "\tjmp .L%s\n", entry_label);
            append_code(env->codes, ".L%s:\n",   exit_label);
            break;
        case AST_DOWHILE_STMT:
            append_code(env->codes, ".L%s:\n",   entry_label);
            gen_stmt_code(ast_nth_child(ast, 0), symbol_table, env);
            gen_expr_code(ast_nth_child(ast, 1), symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            append_code(env->codes, "\tjne .L%s\n", entry_label);
            break;
        case AST_FOR_STMT:
            child = ast_nth_child(ast, 0);
            gen_expr_code(child, symbol_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tadd $8, %%rsp\n");
            }
            append_code(env->codes, ".L%s:\n",   entry_label);
            child = ast_nth_child(ast, 1);
            gen_expr_code(child, symbol_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tpop %%rax\n");
                append_code(env->codes, "\tcmp $0, %%rax\n");
                append_code(env->codes, "\tje .L%s\n", exit_label);
            }
            gen_stmt_code(ast_nth_child(ast, 3), symbol_table, env);
            child = ast_nth_child(ast, 2);
            gen_expr_code(child, symbol_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tadd $8, %%rsp\n");
            }
            append_code(env->codes, "\tjmp .L%s\n", entry_label);
            append_code(env->codes, ".L%s:\n",   exit_label);
            break;
        default:
            assert_code_gen(0);
            break;
    }
    free(entry_label);
    free(exit_label);
}

void gen_jump_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_RETURN_STMT:
            gen_expr_code(ast_nth_child(ast, 0), symbol_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tjmp .L_%s_return\n", env->funcname);
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_stmt_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    AstType type = ast->type;

    if (is_compound_stmt(type)) {
        gen_compound_stmt_code(ast, symbol_table, env);
    } else if (is_expr_stmt(type)) {
        gen_expr_stmt_code(ast, symbol_table, env);
    } else if (is_selection_stmt(type)) {
        gen_selection_stmt_code(ast, symbol_table, env);
    } else if(is_iteration_stmt(type)) {
        gen_iteration_stmt_code(ast, symbol_table, env);
    } else if (is_jump_stmt(type)) {
        gen_jump_stmt_code(ast, symbol_table, env);
    } else  {
        assert_code_gen(0);
    }
}

// declaration-code-generator
void gen_declaration_list_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        gen_declaration_code(ast_nth_child(ast, i), symbol_table, env);
    }
}

void gen_declaration_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    switch(ast->type) {
        case AST_IDENT_DECL:
            lhs = ast_nth_child(ast, 0);
            rhs = ast_nth_child(ast, 1);
            if (rhs->type == AST_NULL) break;
            gen_expr_code(rhs, symbol_table, env);
            gen_lvalue_code(lhs, symbol_table, env);
            append_code(env->codes, "\tpop %%rdi\n");
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tmov %%rax, (%%rdi)\n");
            break;
        case AST_FUNC_DECL:
            /* Do Nothing */
            break;
        default:
            assert_code_gen(0);
    }
}

// external-definition-generator
void gen_function_definition_code(Ast* ast, SymbolTable* symbol_table, CodeEnvironment* env) {
    Ast* function_decl = ast_nth_child(ast, 0);
    Ast* function_ident = ast_nth_child(function_decl, 0);
    Ast* param_list = ast_nth_child(function_decl, 1);
    Ast* block = ast_nth_child(ast, 1);

    free(env->funcname);
    env->funcname = str_new(function_ident->value_ident);

    size_t num_args = param_list->children->size;
    assert_code_gen(num_args <= 6);
    size_t i = 0;
    for (i = 0; i < num_args; i++) {
        Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
        int stack_index = symbol_table_get_stack_index(block->symbol_table, param_ident->value_ident);
        append_code(env->codes, "\tmov %%%s, -%d(%%rbp)\n", arg_register[i], stack_index);
    }
    gen_compound_stmt_code(block, symbol_table, env);

    Vector* codes = vector_new();
    append_code(codes, "\t.global _%s\n", env->funcname);
    append_code(codes, "_%s:\n", env->funcname);
    append_code(codes, "\tpush %%rbp\n");
    append_code(codes, "\tmov %%rsp, %%rbp\n");
    append_code(codes, "\tsub $%d, %%rsp\n", (block->symbol_table->stack_offset + 15) / 16 * 16);
    vector_join(codes, env->codes);
    append_code(codes, ".L_%s_return:\n", env->funcname);
    append_code(codes, "\tmov %%rbp, %%rsp\n");
    append_code(codes, "\tpop %%rbp\n");
    append_code(codes, "\tret\n");
    append_code(codes, "\n");

    vector_delete(env->codes);
    env->codes = codes;
}

// code-environment
CodeEnvironment* code_environment_new() {
    CodeEnvironment* env = (CodeEnvironment*)safe_malloc(sizeof(CodeEnvironment));
    env->funcname = NULL;
    env->num_labels = 0;
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

char* create_new_label(CodeEnvironment* env) {
    if (env->num_labels == 1 << 30) {
        assert_code_gen(0);
        return NULL;
    }
    char* label = safe_malloc(sizeof(char) * (strlen(env->funcname) + 12 + 1));
    sprintf(label, "_%s_%d", env->funcname, env->num_labels);
    env->num_labels++;
    return label;
}

void code_environment_delete(CodeEnvironment* env) {
    free(env->funcname);
    vector_delete(env->codes);
    free(env);
}

// assertion
void assert_code_gen(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to generate code\n");
    exit(1);
}
