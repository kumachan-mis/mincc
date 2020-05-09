#include "semanalyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cast.h"
#include "../common/memory.h"


// expression-semantics-analyzer
void analyze_primary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_postfix_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_unary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_multiplicative_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_additive_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_shift_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_relational_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_equality_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_bitwise_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_logical_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_assignment_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);

// statement-semantics-analyzer
void analyze_compound_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_expr_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_selection_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_iteration_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_jump_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);

// declaration-semantics-analyzer
void analyze_global_declaration_list_semantics(Ast* ast, GlobalList* global_list);
void analyze_global_declaration_semantics(Ast* ast, GlobalList* global_list);
void analyze_local_declaration_list_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
void analyze_local_declaration_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);

// initializer-semantics-analyzer
void analyze_initializer_semantics(
    Ast* init, CType* ctype,
    GlobalList* global_list, LocalTable* local_table
);
void analyze_ident_initializer_semantics(
    Ast* init, CType* ident_ctype,
    GlobalList* global_list, LocalTable* local_table
);
void analyze_array_initializer_semantics(
    Ast* init, CType* array_ctype,
    GlobalList* global_list, LocalTable* local_table
);

// external-declaration-semantics-analyzer
void analyze_function_definition_semantics(Ast* ast, GlobalList* global_list);

// initializer-utils
GlobalData* global_initializer_to_data(Ast* init, CType* ctype);
GlobalData* global_ident_initializer_to_data(Ast* init, CType* ident_ctype);
GlobalData* global_array_initializer_to_data(Ast* init, CType* array_ctype);
void local_initializer_to_completed(Ast* init, CType* ctype);
void local_array_initializer_to_completed(Ast* init, CType* array_ctype);

// assertion
void assert_semantics(int condition);
void unbound_symbol_error(char* symbol_name);


void analyze_semantics(AstList* astlist) {
    GlobalList* global_list = global_list_new();
    astlist->global_list = global_list;
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;

        switch (ast->type) {
            case AST_DECL_LIST:
                analyze_global_declaration_list_semantics(ast, global_list);
                ast_delete(ast);
                astlist_erase_top(astlist);
                break;
            case AST_FUNC_DEF:
                analyze_function_definition_semantics(ast, global_list);
                astlist_pop(astlist);
                break;
            default:
                assert_semantics(0);
        }
    }
    astlist->pos = 0;
}

// expression-semantics-analyzer
void analyze_primary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
     switch (ast->type) {
        case AST_IMM_INT:
            ast->ctype = ctype_new_int();
            break;
        case AST_IMM_STR: {
            Ast* ident = ast_new_ident(AST_IDENT, global_list_create_str_label(global_list));
            ident->ctype = ctype_new_array(ctype_new_char(), strlen(ast->value_str) + 1);

            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            GlobalData* global_data = global_initializer_to_data(ast, ident->ctype);
            global_list_define(global_list, ident->value_ident, global_data);

            ast_move(ast, ident);
            apply_inplace_array_to_ptr_conversion(ast);
            break;
        }
        case AST_IDENT: {
            CType* ctype = local_table_get_ctype(local_table, ast->value_ident);
            if (ctype == NULL) ctype = global_list_get_ctype(global_list, ast->value_ident);
            if (ctype == NULL) unbound_symbol_error(ast->value_ident);
            ast->ctype = ctype_copy(ctype);
            apply_inplace_array_to_ptr_conversion(ast);
            break;
        }
        default:
            assert_semantics(0);
    }
}

void analyze_postfix_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_FUNC_CALL: {
            Ast* callable = ast_nth_child(ast, 0);
            Ast* arg_list = ast_nth_child(ast, 1);
            // TODO: callable object not only an ident
            assert_semantics(callable->type == AST_IDENT);
    
            CType* ctype = local_table_get_ctype(local_table, callable->value_ident);
            if (ctype == NULL) ctype = global_list_get_function_ctype(global_list, callable->value_ident);
            assert_semantics(arg_list->children->size == ctype->func->param_list->size);
    
            size_t i = 0, num_args = arg_list->children->size;
            for (i = 0; i < num_args; i++) {
                Ast* param = ast_nth_child(arg_list, i);
                CType* param_ctype = vector_at(ctype->func->param_list, i);
                analyze_expr_semantics(param, global_list, local_table);
                assert_semantics(ctype_compatible(param->ctype, param_ctype));
            }
            ast->ctype = ctype_copy(ctype->func->ret);
            break;
        }
        case AST_POST_INCR:
        case AST_POST_DECR: {
            Ast* child = ast_nth_child(ast, 0);
            analyze_expr_semantics(child, global_list, local_table);
            assert_semantics(child->type == AST_IDENT || child->type == AST_DEREF);
            ast->ctype = ctype_copy(child->ctype);
            break;
        }
        default:
            assert_semantics(0);
    }
}

void analyze_unary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* child = ast_nth_child(ast, 0);
    analyze_expr_semantics(child, global_list, local_table);

    switch (ast->type) {
        case AST_PRE_INCR:
        case AST_PRE_DECR:
            assert_semantics(child->type == AST_IDENT || child->type == AST_DEREF);
            ast->ctype = ctype_copy(child->ctype);
            break;
        case AST_ADDR:
            revert_inplace_array_to_ptr_conversion(child);
            assert_semantics(child->type == AST_IDENT || child->type == AST_DEREF);
            ast->ctype = ctype_new_ptr(ctype_copy(child->ctype));
            break;
        case AST_DEREF:
            assert_semantics(child->ctype->basic_ctype == CTYPE_PTR);
            ast->ctype = ctype_copy(child->ctype->ptr_to);
            apply_inplace_array_to_ptr_conversion(ast);
            break;
        case AST_POSI:
        case AST_NEGA:
            apply_inplace_integer_promotion(child);
            assert_semantics(child->ctype->basic_ctype == CTYPE_INT);
            ast->ctype = ctype_copy(child->ctype);
            break;
        case AST_NOT:
            apply_inplace_integer_promotion(child);
            assert_semantics(child->ctype->basic_ctype == CTYPE_INT);
            ast->ctype = ctype_copy(child->ctype);
            break;
        case AST_LNOT:
            assert_semantics(child->ctype->basic_ctype == CTYPE_INT);
            ast->ctype = ctype_new_int();
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_multiplicative_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_additive_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    BasicCType lhs_basic_ctype = lhs->ctype->basic_ctype;
    BasicCType rhs_basic_ctype = rhs->ctype->basic_ctype;

    switch (ast->type) {
        case AST_ADD:
            if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_INT
            )
                ast->ctype = ctype_new_int();
            else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_INT
            )
                ast->ctype = ctype_copy(lhs->ctype);
            else if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_PTR
            )
                ast->ctype = ctype_copy(rhs->ctype);
            else
                assert_semantics(0);
            break;
        case AST_SUB:
            if (
                lhs_basic_ctype == CTYPE_INT &&
                rhs_basic_ctype == CTYPE_INT
            )
                ast->ctype = ctype_new_int();
            else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_INT
            )
                ast->ctype = ctype_copy(lhs->ctype);
            else if (
                lhs_basic_ctype == CTYPE_PTR &&
                rhs_basic_ctype == CTYPE_PTR &&
                ctype_equals(lhs->ctype->ptr_to, rhs->ctype->ptr_to)
            )
                ast->ctype = ctype_new_int();
            else
                assert_semantics(0);
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_shift_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_integer_promotion(lhs);
    apply_inplace_integer_promotion(rhs);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_relational_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    ast->ctype = ctype_new_int();
}

void analyze_equality_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    ast->ctype = ctype_new_int();
}

void analyze_bitwise_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_logical_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    ast->ctype = ctype_new_int();
}

void analyze_assignment_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    assert_semantics(
        (lhs->type == AST_IDENT || lhs->type == AST_DEREF) &&
        ctype_compatible(lhs->ctype, rhs->ctype)
    );
    ast->ctype = ctype_copy(lhs->ctype);
}

void analyze_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    AstType type = ast->type;
    if (is_primary_expr(type))             analyze_primary_expr_semantics(ast, global_list, local_table);
    else if (is_postfix_expr(type))        analyze_postfix_expr_semantics(ast, global_list, local_table);
    else if (is_unary_expr(type))          analyze_unary_expr_semantics(ast, global_list, local_table);
    else if (is_multiplicative_expr(type)) analyze_multiplicative_expr_semantics(ast, global_list, local_table);
    else if (is_additive_expr(type))       analyze_additive_expr_semantics(ast, global_list, local_table);
    else if (is_shift_expr(type))          analyze_shift_expr_semantics(ast, global_list, local_table);
    else if (is_relational_expr(type))     analyze_relational_expr_semantics(ast, global_list, local_table);
    else if (is_equality_expr(type))       analyze_equality_expr_semantics(ast, global_list, local_table);
    else if (is_bitwise_expr(type))        analyze_bitwise_expr_semantics(ast, global_list, local_table);
    else if (is_logical_expr(type))        analyze_logical_expr_semantics(ast, global_list, local_table);
    else if (is_assignment_expr(type))     analyze_assignment_expr_semantics(ast, global_list, local_table);
    else if (!is_null_expr(type))          assert_semantics(0);
}

// statement-semantics-analyzer
void analyze_compound_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    LocalTable* block_table = local_table_new(local_table);
    ast->local_table = block_table;

    local_table_enter_into_block_scope(block_table);
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(ast, i);
        if (child->type == AST_DECL_LIST) {
            analyze_local_declaration_list_semantics(child, global_list, block_table);
        } else {
            analyze_stmt_semantics(child, global_list, block_table);
        }
    }
    local_table_exit_from_block_scope(block_table);
}

void analyze_expr_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_EXPR_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), global_list, local_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_selection_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_IF_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), global_list, local_table);
            analyze_stmt_semantics(ast_nth_child(ast, 1), global_list, local_table);
            if (ast->children->size == 2) return;
            analyze_stmt_semantics(ast_nth_child(ast, 2), global_list, local_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_iteration_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_WHILE_STMT: 
            analyze_expr_semantics(ast_nth_child(ast, 0), global_list, local_table);
            analyze_stmt_semantics(ast_nth_child(ast, 1), global_list, local_table);
            break;
        case AST_DOWHILE_STMT:
            analyze_stmt_semantics(ast_nth_child(ast, 0), global_list, local_table);
            analyze_expr_semantics(ast_nth_child(ast, 1), global_list, local_table);
            break;
        case AST_FOR_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), global_list, local_table);
            analyze_expr_semantics(ast_nth_child(ast, 1), global_list, local_table);
            analyze_expr_semantics(ast_nth_child(ast, 2), global_list, local_table);
            analyze_stmt_semantics(ast_nth_child(ast, 3), global_list, local_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_jump_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_RETURN_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), global_list, local_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    AstType type = ast->type;
    if (is_compound_stmt(type))       analyze_compound_stmt_semantics(ast, global_list, local_table);
    else if (is_expr_stmt(type))      analyze_expr_stmt_semantics(ast, global_list, local_table);
    else if (is_selection_stmt(type)) analyze_selection_stmt_semantics(ast, global_list, local_table);
    else if(is_iteration_stmt(type))  analyze_iteration_stmt_semantics(ast, global_list, local_table);
    else if (is_jump_stmt(type))      analyze_jump_stmt_semantics(ast, global_list, local_table);
    else                              assert_semantics(0);
}

// declaration-semantics-analyzer
void analyze_global_declaration_list_semantics(Ast* ast, GlobalList* global_list) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        analyze_global_declaration_semantics(ast_nth_child(ast, i), global_list);
    }
}

void analyze_global_declaration_semantics(Ast* ast, GlobalList* global_list) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* init = NULL;
    GlobalData* global_data = NULL;
    LocalTable* empty_table = local_table_new(NULL);

    switch(ident->ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            if (ast->children->size == 1) {
                global_list_tentatively_define(global_list, ident->value_ident);
                break;
            }
            init = ast_nth_child(ast, 1);
            analyze_initializer_semantics(init, ident->ctype, global_list, empty_table);
            global_data = global_initializer_to_data(init, ident->ctype);
            global_list_define(global_list, ident->value_ident, global_data);
            break;
        case CTYPE_ARRAY:
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            if (ast->children->size == 2) {
                global_list_tentatively_define(global_list, ident->value_ident);
                break;
            }
            init = ast_nth_child(ast, 2);
            analyze_initializer_semantics(init, ident->ctype, global_list, empty_table);
            global_data = global_initializer_to_data(init, ident->ctype);
            global_list_define(global_list, ident->value_ident, global_data);
            break;
        case CTYPE_FUNC:
            apply_inplace_function_declaration_conversion(ast);
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            break;
    }

    local_table_delete(empty_table);
}

void analyze_local_declaration_list_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        analyze_local_declaration_semantics(ast_nth_child(ast, i), global_list, local_table);
    }
}

void analyze_local_declaration_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* init = NULL;

    switch(ident->ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            local_table_define(local_table, ident->value_ident);
            if (ast->children->size == 1) break;
            init = ast_nth_child(ast, 1);
            analyze_initializer_semantics(init, ident->ctype, global_list, local_table);
            local_initializer_to_completed(init, ident->ctype);
            break;
        case CTYPE_ARRAY:
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            local_table_define(local_table, ident->value_ident);
            if (ast->children->size == 2) break;
            init = ast_nth_child(ast, 2);
            analyze_initializer_semantics(init, ident->ctype, global_list, local_table);
            local_initializer_to_completed(init, ident->ctype);
            break;
        case CTYPE_FUNC:
            apply_inplace_function_declaration_conversion(ast);
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            break;
    }
}

// external-declaration-semantics-analyzer
void analyze_function_definition_semantics(Ast* ast, GlobalList* global_list) {
    Ast* func_decl = ast_nth_child(ast, 0);
    Ast* param_list = ast_nth_child(func_decl, 1);
    Ast* block = ast_nth_child(ast, 1);

    apply_inplace_function_declaration_conversion(func_decl);

    Ast* func_ident = ast_nth_child(func_decl, 0);
    global_list_insert_copy(global_list, func_ident->value_ident, func_ident->ctype);
    global_list_define(global_list, func_ident->value_ident, NULL);

    LocalTable* func_table = local_table_new(NULL);
    block->local_table = func_table;

    size_t i = 0, size = param_list->children->size;
    for (i = 0; i < size; i++) {
        Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
        local_table_insert_copy(func_table, param_ident->value_ident, param_ident->ctype);
        local_table_define(func_table, param_ident->value_ident);
    }

    i = 0, size = block->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(block, i);
        if (child->type == AST_DECL_LIST) {
            analyze_local_declaration_list_semantics(child, global_list, func_table);
        } else {
            analyze_stmt_semantics(child, global_list, func_table);
        }
    }
}

// initializer-semantics-analyzer
void analyze_initializer_semantics(
    Ast* init, CType* ctype,
    GlobalList* global_list, LocalTable* local_table
) {
    switch (ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            analyze_ident_initializer_semantics(init, ctype, global_list, local_table);
            break;
        case CTYPE_ARRAY:
            analyze_array_initializer_semantics(init, ctype, global_list, local_table);
            break;
        case CTYPE_FUNC:
            assert_semantics(0);
    }
}

void analyze_ident_initializer_semantics(
    Ast* init, CType* ident_ctype,
    GlobalList* global_list, LocalTable* local_table
) {
    analyze_expr_semantics(init, global_list, local_table);
    assert_semantics(ctype_compatible(ident_ctype, init->ctype));
}

void analyze_array_initializer_semantics(
    Ast* init, CType* array_ctype,
    GlobalList* global_list, LocalTable* local_table
) {
    int array_len = array_ctype->size / array_ctype->array_of->size;
    int initializer_len = 0;
    int i = 0;

    switch (init->type) {
        case AST_INIT_LIST:
            initializer_len = init->children->size;
            assert_semantics(array_len >= initializer_len);
            for (i = 0; i < initializer_len; i++) {
                Ast* child = ast_nth_child(init, i);
                analyze_initializer_semantics(child,  array_ctype->array_of, global_list, local_table);
            }
            break;
        case AST_IMM_STR:
            initializer_len = strlen(init->value_str) + 1;
            assert_semantics(array_len >= initializer_len);
            assert_semantics(array_ctype->array_of->basic_ctype == CTYPE_CHAR);
            break;
        default:
            assert_semantics(0);
    }
}

// initializer-utils
GlobalData* global_initializer_to_data(Ast* init, CType* ctype) {
    GlobalData* global_data = NULL;

    switch (ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            global_data = global_ident_initializer_to_data(init, ctype);
            break;
        case CTYPE_ARRAY:
            global_data = global_array_initializer_to_data(init, ctype);
            break;
        case CTYPE_FUNC:
            assert_semantics(0);
    }
    return global_data;
}

GlobalData* global_ident_initializer_to_data(Ast* init, CType* ident_ctype) {
    GlobalData* global_data = NULL;
    switch (init->type) {
        case AST_IMM_INT:
            global_data = global_data_new_integer(init->value_int, ident_ctype->size);
            break;
        case AST_ADDR: 
        case AST_ARRAY_TO_PTR: {
            Ast* address_of = ast_nth_child(init, 0);
            assert_semantics(address_of->type == AST_IDENT);
            global_data = global_data_new_address(str_new(address_of->value_ident));
            break;
        }
        default:
            assert_semantics(0);
    }
    return global_data;
}

GlobalData* global_array_initializer_to_data(Ast* init, CType* array_ctype) {
    GlobalData* global_data = NULL;

    switch (init->type) {
        case AST_IMM_STR: {
            global_data = global_data_new_list();
            GlobalData* child = NULL;
            child = global_data_new_string(str_new(init->value_str));
            global_data_append_child(global_data, child);
            child = global_data_new_integer(0, array_ctype->size - global_data->size);
            global_data_append_child(global_data, child);
            break;
        }
        case AST_INIT_LIST: {
            global_data = global_data_new_list();
            GlobalData* child = NULL;
            size_t i = 0, size = init->children->size;
            for (i = 0; i < size; i++) {
                child = global_initializer_to_data(ast_nth_child(init, i), array_ctype->array_of);
                global_data_append_child(global_data, child);
            }
            child = global_data_new_integer(0, array_ctype->size - global_data->size);
            global_data_append_child(global_data,  child);
            break;
        }
        default:
            assert_semantics(0);
    }
    return global_data;
}

void local_initializer_to_completed(Ast* init, CType* ctype) {
    switch (ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
            break;
        case CTYPE_ARRAY:
            local_array_initializer_to_completed(init, ctype);
            break;
        case CTYPE_FUNC:
            assert_semantics(0);
    }
}

void local_array_initializer_to_completed(Ast* init, CType* array_ctype) {
    int array_len = array_ctype->size / array_ctype->array_of->size;
    size_t i = 0, size = 0;

    switch (init->type) {
        case AST_IMM_STR: {
            init->type = AST_INIT_LIST;
            size = strlen(init->value_str);
            for (i = 0; i < size; i++) {
                ast_append_child(init, ast_new_int(AST_IMM_INT, init->value_str[i]));
            }
            for (i = size; i < array_len; i++) {
                ast_append_child(init, ast_new_int(AST_IMM_INT, 0));
            }
            break;
        }
        case AST_INIT_LIST:
            size = init->children->size;
            switch (array_ctype->array_of->basic_ctype) {
                case CTYPE_CHAR:
                case CTYPE_INT:
                case CTYPE_PTR:
                    for (i = size; i < array_len; i++) {
                        ast_append_child(init, ast_new_int(AST_IMM_INT, 0));
                    }
                    break;
                case CTYPE_ARRAY:
                    for (i = size; i < array_len; i++) {
                        Ast* child = ast_new(AST_INIT_LIST, 0);
                        local_array_initializer_to_completed(child, array_ctype->array_of);
                        ast_append_child(init, child);
                    }
                    break;
                case CTYPE_FUNC:
                    assert_semantics(0);
            }
            break;
        default:
            assert_semantics(0);
    }
}

// assertion
void assert_semantics(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze semantics\n");
    exit(1);
}

void unbound_symbol_error(char* symbol_name) {
    fprintf(stderr, "Error: unbound symbol '%s'\n", symbol_name);
    exit(1);
}
