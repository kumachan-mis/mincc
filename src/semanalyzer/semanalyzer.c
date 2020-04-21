#include "semanalyzer.h"

#include <stdio.h>
#include <stdlib.h>
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
void analyze_null_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table);
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

// external-declaration-semantics-analyzer
void analyze_function_definition_semantics(Ast* ast, GlobalList* global_list);

// utils
GlobalData* global_initializer_to_data(Ast* init, CType* ctype, GlobalList* global_list);

// assertion
void assert_semantics(int condition);
void unbound_symbol_error(char* symbol_name);


void analyze_semantics(AstList* astlist) {
    GlobalList* global_list = global_list_new();
    astlist->global_list = global_list;
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;
    
        if (is_declaration_list(ast->type)) {
            analyze_global_declaration_list_semantics(ast, global_list);
            ast_delete(ast);
            astlist_erase_top(astlist);
        } else if (ast->type == AST_FUNC_DEF) {
            analyze_function_definition_semantics(ast, global_list);
            astlist_pop(astlist);
        } else {
            assert_semantics(0);
        }
    }
    astlist->pos = 0;
}

// expression-semantics-analyzer
void analyze_primary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    CType* ctype = NULL;

     switch (ast->type) {
        case AST_IMM_INT:
            ast->ctype = ctype_new_int();
            break;
        case AST_IDENT:
            ctype = local_table_get_ctype(local_table, ast->value_ident);
            if (ctype == NULL) ctype = global_list_get_ctype(global_list, ast->value_ident);
            if (ctype == NULL) unbound_symbol_error(ast->value_ident);
            ast->ctype = ctype_copy(ctype);
            apply_inplace_array_to_ptr_conversion(ast);
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_postfix_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    CType* ctype = NULL;

    switch (ast->type) {
        case AST_FUNC_CALL:
            // TODO: callable object not only an ident
            assert_semantics(lhs->type == AST_IDENT);
            ctype = local_table_get_ctype(local_table, lhs->value_ident);
            if (ctype == NULL) ctype = global_list_get_function_ctype(global_list, lhs->value_ident);
            assert_semantics(rhs->children->size == ctype->func->param_types->size);
            size_t i = 0, num_args = rhs->children->size;
            for (i = 0; i < num_args; i++) {
                Ast* param = ast_nth_child(rhs, i);
                CType* param_ctype = vector_at(ctype->func->param_types, i);
                analyze_expr_semantics(param, global_list, local_table);
                assert_semantics(ctype_compatible(param->ctype, param_ctype));
            }
            ast->ctype = ctype_copy(ctype->func->return_type);
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_unary_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* child = ast_nth_child(ast, 0);
    analyze_expr_semantics(child, global_list, local_table);

    switch (ast->type) {
        case AST_ADDR:
            assert_semantics(child->type == AST_IDENT || child->type == AST_DEREF);
            ast->ctype = ctype_new_ptr(ctype_copy(child->ctype));
            break;
        case AST_DEREF:
            assert_semantics(child->ctype->basic_ctype == CTYPE_PTR);
            ast->ctype = ctype_copy(child->ctype->ptr_to);
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
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    // TODO: not only int
    ast->ctype = ctype_new_int();
}

void analyze_equality_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    // TODO: not only int
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
    assert_semantics(lhs->type == AST_IDENT || lhs->type == AST_DEREF);
    analyze_expr_semantics(lhs, global_list, local_table);
    analyze_expr_semantics(rhs, global_list, local_table);
    assert_semantics(ctype_compatible(lhs->ctype, rhs->ctype));
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
    else if (is_null_expr(type))           analyze_null_expr_semantics(ast, global_list, local_table);
    else                                   assert_semantics(0);
}

void analyze_null_expr_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    switch (ast->type) {
        case AST_NULL:
            // Do Nothing
            break;
        default:
            assert_semantics(0);
    }
}

// statement-semantics-analyzer
void analyze_compound_stmt_semantics(Ast* ast, GlobalList* global_list, LocalTable* local_table) {
    LocalTable* block_table = local_table_new(local_table);
    ast->local_table = block_table;

    local_table_enter_into_block_scope(block_table);
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(ast, i);
        if (is_declaration_list(child->type)) {
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

    switch(ast->type) {
        case AST_IDENT_DECL:
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            if (ast->children->size == 1) {
                global_list_tentatively_define(global_list, ident->value_ident);
            } else {
                Ast* init = ast_nth_child(ast, 1);
                GlobalData* global_data = global_initializer_to_data(init, ident->ctype, global_list);
                global_list_define(global_list, ident->value_ident, global_data);
            }
            break;
        case AST_ARRAY_DECL:
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            if (ast->children->size == 1) {
                global_list_tentatively_define(global_list, ident->value_ident);
            } else {
                assert_semantics(0);
            }
            break;
        case AST_FUNC_DECL:
            apply_inplace_function_declaration_conversion(ast);
            global_list_insert_copy(global_list, ident->value_ident, ident->ctype);
            break;
        default:
            assert_semantics(0);
    }
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

    switch(ast->type) {
        case AST_IDENT_DECL:
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            local_table_define(local_table, ident->value_ident);
            if (ast->children->size == 1) break;
            init = ast_nth_child(ast, 1);
            analyze_expr_semantics(init, global_list, local_table);
            assert_semantics(ctype_compatible(ident->ctype, init->ctype));
            break;
        case AST_ARRAY_DECL:
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            local_table_define(local_table, ident->value_ident);
            if (ast->children->size == 1) break;
            assert_semantics(0);
            // TODO: initializer of arrays
            break;
        case AST_FUNC_DECL:
            apply_inplace_function_declaration_conversion(ast);
            local_table_insert_copy(local_table, ident->value_ident, ident->ctype);
            break;
        default:
            assert_semantics(0);
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
        if (is_declaration_list(child->type)) {
            analyze_local_declaration_list_semantics(child, global_list, func_table);
        } else {
            analyze_stmt_semantics(child, global_list, func_table);
        }
    }
}

// utils
GlobalData* global_initializer_to_data(Ast* init, CType* ctype, GlobalList* global_list) {
    GlobalData* global_data = global_data_new();

    if (init->type == AST_IMM_INT) {
        init->ctype = ctype_new_int();
        assert_semantics(ctype_compatible(ctype, init->ctype));
        global_data_append_integer(global_data, init->value_int, ctype->size);
        global_data_set_zero_size(global_data, 0);
        return global_data;
    }

    Ast* address_of = ast_nth_child(init, 0);
    if (init->type == AST_ADDR && address_of->type == AST_IDENT) {
        CType* address_of_ctype = global_list_get_ctype(global_list, address_of->value_ident);
        if (address_of_ctype == NULL) unbound_symbol_error(address_of->value_ident);
        address_of->ctype = ctype_copy(address_of_ctype);
        init->ctype = ctype_new_ptr(ctype_copy(address_of->ctype));
        assert_semantics(ctype_compatible(ctype, init->ctype));
        global_data_append_address(global_data, str_new(address_of->value_ident));
        global_data_set_zero_size(global_data, 0);
        return global_data;
    }

    assert_semantics(0);
    return global_data;
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
