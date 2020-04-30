#include "gen.h"

#include <stdlib.h>
#include <string.h>
#include "codenv.h"
#include "../parser/localtable.h"
#include "../common/memory.h"


char* arg_register1[] = { "%dil", "%sil", "%dl",  "%cl",  "%r8b", "%r9b" };
char *arg_register4[] = { "%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d" };
char* arg_register8[] = { "%rdi", "%rsi", "%rdx", "%rcx", "%r8",  "%r9"  };


void put_code(FILE* file_ptr, Vector* codes);

// expression-code-generator
void gen_primary_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_postfix_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_unary_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_multiplicative_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_cast_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_additive_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_shift_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_relational_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_equality_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_bitwise_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_logical_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_assignment_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env);

// statement-code-generator
void gen_compound_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_expr_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_selection_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_iteration_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_jump_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env);

// declaration-code-generator
void gen_declaration_list_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_declaration_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_ident_initialization_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_array_initialization_code(Ast* ast, LocalTable* local_table, CodeEnv* env);

// external-declaration-generator
void gen_global_variable_code(GlobalVariable* gloval_variable, Vector* codes);
void gen_function_definition_code(Ast* ast, Vector* codes);

// utils
void gen_address_code(Ast* ast, LocalTable* local_table, CodeEnv* env);
void gen_load_code(CType* ctype, CodeEnv* env);
void gen_store_code(CType* ctype, CodeEnv* env);
void gen_store_arg_code(int arg_index, CType* ctype, CodeEnv* env);
void gen_inc_code(CType* ctype, CodeEnv* env);
void gen_dec_code(CType* ctype, CodeEnv* env);
char* create_size_label(int size);

// assertion
void assert_code_gen(int condition);


void print_code(FILE* file_ptr, AstList* astlist) {
    Vector* codes = vector_new();

    while (1) {
        GlobalVariable* global_variable = global_list_top(astlist->global_list);
        if (global_variable == NULL) break;
        gen_global_variable_code(global_variable, codes);
        global_list_pop(astlist->global_list);
    }
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;
        gen_function_definition_code(ast, codes);
        astlist_pop(astlist);
    }

    put_code(file_ptr, codes);
    vector_delete(codes);
    astlist->global_list->pos = 0;
    astlist->pos = 0;
}

void put_code(FILE* file_ptr, Vector* codes) {
    size_t i = 0, size = codes->size;
    for (i = 0; i < size; i++) {
        char* str = (char*)vector_at(codes, i);
        fputs(str, file_ptr);
    }
}

// expression-code-generator
void gen_primary_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    switch (ast->type) {
        case AST_IMM_INT:
            append_code(env->codes, "\tpush $%d\n", ast->value_int);
            break;
        case AST_IDENT:
            gen_address_code(ast, local_table, env);
            gen_load_code(ast->ctype, env);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_postfix_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    switch (ast->type) {
        case AST_FUNC_CALL: {
            Ast* callable = ast_nth_child(ast, 0);
            Ast* arg_list = ast_nth_child(ast, 1);
            // TODO: callable object not only an ident
            assert_code_gen(callable->type == AST_IDENT);

            size_t i = 0, num_args = arg_list->children->size;
            // TODO: more than six arguments
            assert_code_gen(num_args <= 6);
            for (i = 0; i < num_args; i++) {
                gen_expr_code(ast_nth_child(arg_list, i), local_table, env);
            }
            for (i = 0; i < num_args; i++) {
                append_code(env->codes, "\tpop %s\n", arg_register8[num_args - i - 1]);
            }
            append_code(env->codes, "\tcall _%s\n", callable->value_ident);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        }
        case AST_POST_INCR:
        case AST_POST_DECR: {
            Ast* child = ast_nth_child(ast, 0);
            gen_address_code(child, local_table, env);
            append_code(env->codes, "\tmov %%rax, %%rdi\n");
            gen_load_code(child->ctype, env);
            append_code(env->codes, "\tpush %%rax\n");
            if (ast->type == AST_POST_INCR) gen_inc_code(child->ctype, env);
            else                            gen_dec_code(child->ctype, env);
            gen_store_code(ast->ctype, env);
            break;
        }
        default:
            assert_code_gen(0);
    }
}

void gen_unary_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* child = ast_nth_child(ast, 0);

    switch (ast->type) {
        case AST_PRE_INCR:
        case AST_PRE_DECR:
            gen_address_code(child, local_table, env);
            append_code(env->codes, "\tmov %%rax, %%rdi\n");
            gen_load_code(child->ctype, env);
            if (ast->type == AST_PRE_INCR) gen_inc_code(child->ctype, env);
            else                           gen_dec_code(child->ctype, env);
            append_code(env->codes, "\tpush %%rax\n");
            gen_store_code(ast->ctype, env);
            break;
        case AST_ADDR:
            gen_address_code(child, local_table, env);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_DEREF:
            gen_expr_code(child, local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            gen_load_code(ast->ctype, env);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_POSI:
            gen_expr_code(child, local_table, env);
            break;
        case AST_NEGA:
            gen_expr_code(child, local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tneg %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_NOT:
            gen_expr_code(child, local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tnot %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_LNOT:
            gen_expr_code(child, local_table, env);
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

void gen_cast_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* child = ast_nth_child(ast, 0);

    switch (ast->type) {
        case AST_ARRAY_TO_PTR:
            gen_address_code(child, local_table, env);
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);  
    }
}

void gen_multiplicative_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
    gen_expr_code(ast_nth_child(ast, 1), local_table, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_MUL:
            append_code(env->codes, "\timul %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_DIV:
            append_code(env->codes, "\tcdq\n");
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

void gen_additive_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    gen_expr_code(lhs, local_table, env);
    gen_expr_code(rhs, local_table, env);
    BasicCType lhs_basic_ctype = lhs->ctype->basic_ctype;
    BasicCType rhs_basic_ctype = rhs->ctype->basic_ctype;

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_ADD:
            if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_INT
            ) {
                append_code(env->codes, "\tadd %%edi, %%eax\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_INT
            ) {
                append_code(env->codes, "\timul $%d, %%rdi\n", lhs->ctype->ptr_to->size);
                append_code(env->codes, "\tadd %%rdi, %%rax\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_PTR
            ) {
                append_code(env->codes, "\timul $%d, %%rax\n", rhs->ctype->ptr_to->size);
                append_code(env->codes, "\tadd %%rdi, %%rax\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else {
                assert_code_gen(0);
            }
            break;
        case AST_SUB:
            if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_INT
            ) {
                append_code(env->codes, "\tsub %%edi, %%eax\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_INT
            ) {
                append_code(env->codes, "\timul $%d, %%rdi\n", lhs->ctype->ptr_to->size);
                append_code(env->codes, "\tsub %%rdi, %%rax\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_PTR &&
                ctype_equals(lhs->ctype->ptr_to, rhs->ctype->ptr_to)
            ) {
                append_code(env->codes, "\tsub %%rdi, %%rax\n");
                append_code(env->codes, "\tmov $%d, %%rdi\n", lhs->ctype->ptr_to->size);
                append_code(env->codes, "\tcqo\n");
                append_code(env->codes, "\tidiv %%rdi\n");
                append_code(env->codes, "\tpush %%rax\n");
            } else {
                assert_code_gen(0);
            }
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_shift_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
    gen_expr_code(ast_nth_child(ast, 1), local_table, env);

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

void gen_equality_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
    gen_expr_code(ast_nth_child(ast, 1), local_table, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->ctype->size) {
        case 4:
            append_code(env->codes, "\tcmp %%edi, %%eax\n");
            break;
        case 8:
            append_code(env->codes, "\tcmp %%rdi, %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
    switch (ast->type) {
        case AST_EQ:
            append_code(env->codes, "\tsete %%al\n");
            break;
        case AST_NEQ:
            append_code(env->codes, "\tsetne %%al\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_relational_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
    gen_expr_code(ast_nth_child(ast, 1), local_table, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->ctype->size) {
        case 4:
            append_code(env->codes, "\tcmp %%edi, %%eax\n");
            break;
        case 8:
            append_code(env->codes, "\tcmp %%rdi, %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
    switch (ast->type) {
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

void gen_bitwise_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
    gen_expr_code(ast_nth_child(ast, 1), local_table, env);

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

void gen_logical_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    char* exit_label = codenv_create_label(env);

    gen_expr_code(ast_nth_child(ast, 0), local_table, env);
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

    gen_expr_code(ast_nth_child(ast, 1), local_table, env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    append_code(env->codes, ".L%s:\n", exit_label);
    append_code(env->codes, "\tsetne %%al\n");
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");

    free(exit_label);
}

void gen_assignment_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* expr = ast_nth_child(ast, 1);
    gen_expr_code(expr, local_table, env);
    gen_address_code(ident, local_table, env);

    append_code(env->codes, "\tmov %%rax, %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");

    switch (ast->type) {
        case AST_ASSIGN:
            break;
        default:
            assert_code_gen(0);
    }

    gen_store_code(ident->ctype, env);
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_expr_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    AstType type = ast->type;
    if (is_primary_expr(type))             gen_primary_expr_code(ast, local_table, env);
    else if (is_postfix_expr(type))        gen_postfix_expr_code(ast, local_table, env);
    else if (is_unary_expr(type))          gen_unary_expr_code(ast, local_table, env);
    else if (is_cast_expr(type))           gen_cast_expr_code(ast, local_table, env);
    else if (is_multiplicative_expr(type)) gen_multiplicative_expr_code(ast, local_table, env);
    else if (is_additive_expr(type))       gen_additive_expr_code(ast, local_table, env);
    else if (is_shift_expr(type))          gen_shift_expr_code(ast, local_table, env);
    else if (is_relational_expr(type))     gen_relational_expr_code(ast, local_table, env);
    else if (is_equality_expr(type))       gen_equality_expr_code(ast, local_table, env);
    else if (is_bitwise_expr(type))        gen_bitwise_expr_code(ast, local_table, env);
    else if (is_logical_expr(type))        gen_logical_expr_code(ast, local_table, env);
    else if (is_assignment_expr(type))     gen_assignment_expr_code(ast, local_table, env);
    else if (!is_null_expr(type))          assert_code_gen(0);  
}

// statement-code-generator
void gen_compound_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    size_t i = 0, size = ast->children->size;
    switch (ast->type) {
        case AST_COMP_STMT:
            for (i = 0; i < size; i++) {
                Ast* child = ast_nth_child(ast, i);
                if (child->type == AST_DECL_LIST) {
                    gen_declaration_list_code(child, ast->local_table, env);
                } else {
                    gen_stmt_code(child, ast->local_table, env);
                }
            }
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_expr_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* child = NULL;

    switch (ast->type) {
        case AST_EXPR_STMT:
            child = ast_nth_child(ast, 0);
            gen_expr_code(child, local_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tadd $8, %%rsp\n");
            }
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_selection_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    switch (ast->type) {
        case AST_IF_STMT:
            gen_expr_code(ast_nth_child(ast, 0), local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            if (ast->children->size == 2) {
                char* exit_label = codenv_create_label(env);
                append_code(env->codes, "\tje .L%s\n", exit_label);
                gen_stmt_code(ast_nth_child(ast, 1), local_table, env);
                append_code(env->codes, ".L%s:\n",   exit_label);
                free(exit_label);
            } else {
                char* else_label = codenv_create_label(env);
                char* exit_label = codenv_create_label(env);
                append_code(env->codes, "\tje .L%s\n", else_label);
                gen_stmt_code(ast_nth_child(ast, 1), local_table, env);
                append_code(env->codes, "\tjmp .L%s\n", exit_label);
                append_code(env->codes, ".L%s:\n",   else_label);
                gen_stmt_code(ast_nth_child(ast, 2), local_table, env);
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

void gen_iteration_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* child = NULL;
    char* entry_label = codenv_create_label(env);
    char* exit_label = codenv_create_label(env);

    switch (ast->type) {
        case AST_WHILE_STMT: 
            append_code(env->codes, ".L%s:\n",   entry_label);
            gen_expr_code(ast_nth_child(ast, 0), local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            append_code(env->codes, "\tje .L%s\n", exit_label);
            gen_stmt_code(ast_nth_child(ast, 1), local_table, env);
            append_code(env->codes, "\tjmp .L%s\n", entry_label);
            append_code(env->codes, ".L%s:\n",   exit_label);
            break;
        case AST_DOWHILE_STMT:
            append_code(env->codes, ".L%s:\n",   entry_label);
            gen_stmt_code(ast_nth_child(ast, 0), local_table, env);
            gen_expr_code(ast_nth_child(ast, 1), local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%rax\n");
            append_code(env->codes, "\tjne .L%s\n", entry_label);
            break;
        case AST_FOR_STMT:
            child = ast_nth_child(ast, 0);
            gen_expr_code(child, local_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tadd $8, %%rsp\n");
            }
            append_code(env->codes, ".L%s:\n",   entry_label);
            child = ast_nth_child(ast, 1);
            gen_expr_code(child, local_table, env);
            if (!is_null_expr(child->type)) {
                append_code(env->codes, "\tpop %%rax\n");
                append_code(env->codes, "\tcmp $0, %%rax\n");
                append_code(env->codes, "\tje .L%s\n", exit_label);
            }
            gen_stmt_code(ast_nth_child(ast, 3), local_table, env);
            child = ast_nth_child(ast, 2);
            gen_expr_code(child, local_table, env);
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

void gen_jump_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    switch (ast->type) {
        case AST_RETURN_STMT:
            gen_expr_code(ast_nth_child(ast, 0), local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tjmp .L_%s_return\n", env->funcname);
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_stmt_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    AstType type = ast->type;
    if (is_compound_stmt(type))       gen_compound_stmt_code(ast, local_table, env);
    else if (is_expr_stmt(type))      gen_expr_stmt_code(ast, local_table, env);
    else if (is_selection_stmt(type)) gen_selection_stmt_code(ast, local_table, env);
    else if(is_iteration_stmt(type))  gen_iteration_stmt_code(ast, local_table, env);
    else if (is_jump_stmt(type))      gen_jump_stmt_code(ast, local_table, env);
    else                              assert_code_gen(0);  
}

// declaration-code-generator
void gen_declaration_list_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        gen_declaration_code(ast_nth_child(ast, i), local_table, env);
    }
}

void gen_declaration_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* ident = ast_nth_child(ast, 0);
    switch(ident->ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            if (ast->children->size == 1) break;
            gen_ident_initialization_code(ast, local_table, env);
            break;
        case CTYPE_ARRAY:
            if (ast->children->size == 1) break;
            gen_array_initialization_code(ast, local_table, env);
            break;
        case CTYPE_FUNC:
            // Do Nothing
            break;
    }
}

void gen_ident_initialization_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* init = ast_nth_child(ast, 1);
    gen_expr_code(init, local_table, env);
    gen_address_code(ident, local_table, env);

    append_code(env->codes, "\tmov %%rax, %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    gen_store_code(ident->ctype, env);
}

void gen_array_initialization_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* init = ast_nth_child(ast, 1);

    size_t i = 0, size = init->children->size;
    CType* array_of = ident->ctype->array_of;
    for (i = 0; i < size; i++) {
        gen_expr_code(ast_nth_child(init, i), local_table, env);
        if (i == 0) {
            gen_address_code(ident, local_table, env);
            append_code(env->codes, "\tmov %%rax, %%rdi\n");
        } else {
            append_code(env->codes, "\tadd $%d, %%rdi\n", array_of->size);
        }
        append_code(env->codes, "\tpop %%rax\n");
        gen_store_code(array_of, env);
    }
}

// external-declaration-generator
void gen_global_variable_code(GlobalVariable* gloval_variable, Vector* codes) {
    if (
        gloval_variable->ctype->basic_ctype == CTYPE_FUNC ||
        gloval_variable->status == GLOBAL_SYMBOL_DECL
    )
        return;

    char* variable_name = gloval_variable->symbol_name;
    int align = 2;
    append_code(codes, "\t.text\n");
    append_code(codes, "\t.global _%s\n", variable_name);
    append_code(codes, "\t.data\n");
    append_code(codes, "\t.align %d\n", align);
    append_code(codes, "_%s:\n", variable_name);

    GlobalData* global_data = gloval_variable->global_data;
    size_t i = 0, size = global_data->inner_vector->size;
    for (i = 0; i < size; i++) {
        GlobalDatum* datum = global_data_nth_datum(global_data, i);
        char* size_label = NULL;
        switch (datum->type) {
            case GBL_TYPE_INTEGER:
                size_label = create_size_label(datum->size);
                append_code(codes, "\t%s %d\n", size_label, datum->value_int);
                free(size_label);
                break;
            case GBL_TYPE_ADDR:
                size_label = create_size_label(datum->size);
                append_code(codes, "\t%s _%s\n", size_label, datum->address_of);
                free(size_label);
                break;
            case GBL_TYPE_STR:
                append_code(codes, "\t.ascii \"%s\\0\"\n", datum->value_str);
                break;
        }
    }

    if (global_data->zero_size > 0) {
        append_code(codes, "\t.zero %d\n", global_data->zero_size);
    }
}

void gen_function_definition_code(Ast* ast, Vector* codes) {
    Ast* func_decl = ast_nth_child(ast, 0);
    Ast* param_list = ast_nth_child(func_decl, 1);
    Ast* block = ast_nth_child(ast, 1);

    Ast* func_ident = ast_nth_child(func_decl, 0);
    CodeEnv* env = codenv_new(str_new(func_ident->value_ident));

    size_t i = 0, size = param_list->children->size;
    // TODO: more than six arguments
    assert_code_gen(size <= 6);

    for (i = 0; i < size; i++) {
        Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
        gen_address_code(param_ident, block->local_table, env);
        gen_store_arg_code(i, param_ident->ctype, env);
    }

    i = 0, size = block->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(block, i);
        if (child->type == AST_DECL_LIST) {
            gen_declaration_list_code(child, block->local_table, env);
        } else {
            gen_stmt_code(child, block->local_table, env);
        }
    }

    append_code(codes, "\t.text\n");
    append_code(codes, "\t.global _%s\n", env->funcname);
    append_code(codes, "_%s:\n", env->funcname);
    append_code(codes, "\tpush %%rbp\n");
    append_code(codes, "\tmov %%rsp, %%rbp\n");
    append_code(codes, "\tsub $%d, %%rsp\n", (block->local_table->stack_offset + 15) / 16 * 16);
    vector_join(codes, env->codes);
    append_code(codes, ".L_%s_return:\n", env->funcname);
    append_code(codes, "\tmov %%rbp, %%rsp\n");
    append_code(codes, "\tpop %%rbp\n");
    append_code(codes, "\tret\n");

    codenv_delete(env);
}

// utils
void gen_address_code(Ast* ast, LocalTable* local_table, CodeEnv* env) {
    int stack_index = 0;

    switch(ast->type) {
        case AST_IDENT:
            stack_index = local_table_get_stack_index(local_table, ast->value_ident);
            if (stack_index >= 0) {
                append_code(env->codes, "\tlea -%d(%%rbp), %%rax\n", stack_index);
            } else {
                append_code(env->codes, "\tlea _%s(%%rip), %%rax\n", ast->value_ident);
            }
            break;
        case AST_DEREF:
            gen_expr_code(ast_nth_child(ast, 0), local_table, env);
            append_code(env->codes, "\tpop %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_load_code(CType* ctype, CodeEnv* env) {
    switch (ctype->size) {
        case 1:
            append_code(env->codes, "\tmovsbl (%%rax), %%eax\n");
            break;
        case 4:
            append_code(env->codes, "\tmov (%%rax), %%eax\n");
            break;
        case 8:
            append_code(env->codes, "\tmov (%%rax), %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_store_code(CType* ctype, CodeEnv* env) {
    switch (ctype->size) {
        case 1:
            append_code(env->codes, "\tmov %%al, (%%rdi)\n");
            break;
        case 4:
            append_code(env->codes, "\tmov %%eax, (%%rdi)\n");
            break;
        case 8:
            append_code(env->codes, "\tmov %%rax, (%%rdi)\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_store_arg_code(int arg_index, CType* ctype, CodeEnv* env) {
    switch (ctype->size) {
        case 1:
            append_code(env->codes, "\tmov %s, (%%rax)\n", arg_register1[arg_index]);
            break;
        case 4:
            append_code(env->codes, "\tmov %s, (%%rax)\n", arg_register4[arg_index]);
            break;
        case 8:
            append_code(env->codes, "\tmov %s, (%%rax)\n", arg_register8[arg_index]);
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_inc_code(CType* ctype, CodeEnv* env) {
    if (ctype_is_integer_ctype(ctype)) {
        append_code(env->codes, "\tinc %%eax\n");
    } else if (ctype->basic_ctype == CTYPE_PTR) {
        append_code(env->codes, "\tadd $%d, %%rax\n", ctype->ptr_to->size);
    } else {
        assert_code_gen(0);
    }
}

void gen_dec_code(CType* ctype, CodeEnv* env) {
    if (ctype_is_integer_ctype(ctype)) {
        append_code(env->codes, "\tdec %%eax\n");
    } else if (ctype->basic_ctype == CTYPE_PTR) {
        append_code(env->codes, "\tsub $%d, %%rax\n", ctype->ptr_to->size);
    } else {
        assert_code_gen(0);
    }
}

char* create_size_label(int size) {
    char* size_label = (char*)safe_malloc(7 * sizeof(char));
    switch (size) {
        case 1:
            strcpy(size_label, ".byte");
            break;
        case 2:
            strcpy(size_label, ".value");
            break;
        case 4:
            strcpy(size_label, ".long");
            break;
        case 8:
            strcpy(size_label, ".quad");
            break;
        default:
            assert_code_gen(0);
    }
    return size_label;
}

// assertion
void assert_code_gen(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to generate code\n");
    exit(1);
}
