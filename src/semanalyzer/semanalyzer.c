#include "semanalyzer.h"

#include <stdio.h>
#include <stdlib.h>
#include "cast.h"
#include "../common/memory.h"


// expression-semantics-analyzer
void analyze_primary_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_postfix_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_unary_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_multiplicative_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_additive_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_shift_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_relational_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_equality_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_bitwise_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_logical_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_assignment_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_null_expr_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_expr_semantics(Ast* ast, SymbolTable* symbol_table);

// statement-semantics-analyzer
void analyze_compound_stmt_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_expr_stmt_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_selection_stmt_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_iteration_stmt_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_jump_stmt_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_stmt_semantics(Ast* ast, SymbolTable* symbol_table);

// declaration-semantics-analyzer
void analyze_global_declaration_list_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_global_declaration_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_local_declaration_list_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_local_declaration_semantics(Ast* ast, SymbolTable* symbol_table);

// external-declaration-semantics-analyzer
void analyze_external_declaration_semantics(Ast* ast, SymbolTable* symbol_table);
void analyze_function_definition_semantics(Ast* ast, SymbolTable* symbol_table);

// assertion
void assert_semantics(int condition);


void analyze_semantics(AstList* astlist) {
    SymbolTable* global_table = symbol_table_new(NULL);
    astlist->symbol_table = global_table;
    while (1) {
        Ast* ast = astlist_top(astlist);
        if (ast == NULL) break;

        analyze_external_declaration_semantics(ast, global_table);
        astlist_pop(astlist);
    }
    astlist->pos = 0;
}

// expression-semantics-analyzer
void analyze_primary_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    CType* ctype = NULL;

     switch (ast->type) {
        case AST_IMM_INT:
            ast->ctype = ctype_new_int();
            break;
        case AST_IDENT:
            ctype = symbol_table_get_ctype(symbol_table, ast->value_ident);
            ast->ctype = ctype_copy(ctype);
            apply_inplace_array_to_ptr_conversion(ast);
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_postfix_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    CType* ctype = NULL;

    switch (ast->type) {
        case AST_FUNC_CALL:
            // TODO: callable object not only an ident
            assert_semantics(lhs->type == AST_IDENT);

            ctype = symbol_table_get_function_ctype(symbol_table, lhs->value_ident);
            assert_semantics(rhs->children->size == ctype->func->param_types->size);
            size_t i = 0, num_args = rhs->children->size;
            for (i = 0; i < num_args; i++) {
                Ast* param = ast_nth_child(rhs, i);
                CType* param_ctype = vector_at(ctype->func->param_types, i);
                analyze_expr_semantics(param, symbol_table);
                assert_semantics(ctype_compatible(param->ctype, param_ctype));
            }
            ast->ctype = ctype_copy(ctype->func->return_type);
            break;
        default:
            assert_semantics(0);
    }
}

void analyze_unary_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* child = ast_nth_child(ast, 0);
    analyze_expr_semantics(child, symbol_table);

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

void analyze_multiplicative_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_additive_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
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

void analyze_shift_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    apply_inplace_integer_promotion(lhs);
    apply_inplace_integer_promotion(rhs);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_relational_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    // TODO: not only int
    ast->ctype = ctype_new_int();
}

void analyze_equality_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    // TODO: not only int
    ast->ctype = ctype_new_int();
}

void analyze_bitwise_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    apply_inplace_usual_arithmetic_conversion(ast);
    assert_semantics(lhs->ctype->basic_ctype == CTYPE_INT);
    assert_semantics(rhs->ctype->basic_ctype == CTYPE_INT);
    ast->ctype = ctype_new_int();
}

void analyze_logical_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    ast->ctype = ctype_new_int();
}

void analyze_assignment_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* lhs = ast_nth_child(ast, 0);
    Ast* rhs = ast_nth_child(ast, 1);
    assert_semantics(lhs->type == AST_IDENT || lhs->type == AST_DEREF);
    analyze_expr_semantics(lhs, symbol_table);
    analyze_expr_semantics(rhs, symbol_table);
    assert_semantics(ctype_compatible(lhs->ctype, rhs->ctype));
    ast->ctype = ctype_copy(lhs->ctype);
}

void analyze_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    AstType type = ast->type;
    if (is_primary_expr(type))             analyze_primary_expr_semantics(ast, symbol_table);
    else if (is_postfix_expr(type))        analyze_postfix_expr_semantics(ast, symbol_table);
    else if (is_unary_expr(type))          analyze_unary_expr_semantics(ast, symbol_table);
    else if (is_multiplicative_expr(type)) analyze_multiplicative_expr_semantics(ast, symbol_table);
    else if (is_additive_expr(type))       analyze_additive_expr_semantics(ast, symbol_table);
    else if (is_shift_expr(type))          analyze_shift_expr_semantics(ast, symbol_table);
    else if (is_relational_expr(type))     analyze_relational_expr_semantics(ast, symbol_table);
    else if (is_equality_expr(type))       analyze_equality_expr_semantics(ast, symbol_table);
    else if (is_bitwise_expr(type))        analyze_bitwise_expr_semantics(ast, symbol_table);
    else if (is_logical_expr(type))        analyze_logical_expr_semantics(ast, symbol_table);
    else if (is_assignment_expr(type))     analyze_assignment_expr_semantics(ast, symbol_table);
    else if (is_null_expr(type))           analyze_null_expr_semantics(ast, symbol_table);
    else                                   assert_semantics(0);
}

void analyze_null_expr_semantics(Ast* ast, SymbolTable* symbol_table) {
    switch (ast->type) {
        case AST_NULL:
            // Do Nothing
            break;
        default:
            assert_semantics(0);
    }
}

// statement-semantics-analyzer
void analyze_compound_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    SymbolTable* block_table = symbol_table_new(symbol_table);
    ast->symbol_table = block_table;

    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(ast, i);
        if (is_declaration_list(child->type)) {
            analyze_local_declaration_list_semantics(child, block_table);
        } else {
            analyze_stmt_semantics(child, block_table);
        }
    }
}

void analyze_expr_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    switch (ast->type) {
        case AST_EXPR_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), symbol_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_selection_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    switch (ast->type) {
        case AST_IF_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), symbol_table);
            analyze_stmt_semantics(ast_nth_child(ast, 1), symbol_table);
            if (ast->children->size == 2) return;
            analyze_stmt_semantics(ast_nth_child(ast, 2), symbol_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_iteration_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    switch (ast->type) {
        case AST_WHILE_STMT: 
            analyze_expr_semantics(ast_nth_child(ast, 0), symbol_table);
            analyze_stmt_semantics(ast_nth_child(ast, 1), symbol_table);
            break;
        case AST_DOWHILE_STMT:
            analyze_stmt_semantics(ast_nth_child(ast, 0), symbol_table);
            analyze_expr_semantics(ast_nth_child(ast, 1), symbol_table);
            break;
        case AST_FOR_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), symbol_table);
            analyze_expr_semantics(ast_nth_child(ast, 1), symbol_table);
            analyze_expr_semantics(ast_nth_child(ast, 2), symbol_table);
            analyze_stmt_semantics(ast_nth_child(ast, 3), symbol_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_jump_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    switch (ast->type) {
        case AST_RETURN_STMT:
            analyze_expr_semantics(ast_nth_child(ast, 0), symbol_table);
            break;
        default:
            assert_semantics(0);
            break;
    }
}

void analyze_stmt_semantics(Ast* ast, SymbolTable* symbol_table) {
    AstType type = ast->type;
    if (is_compound_stmt(type))       analyze_compound_stmt_semantics(ast, symbol_table);
    else if (is_expr_stmt(type))      analyze_expr_stmt_semantics(ast, symbol_table);
    else if (is_selection_stmt(type)) analyze_selection_stmt_semantics(ast, symbol_table);
    else if(is_iteration_stmt(type))  analyze_iteration_stmt_semantics(ast, symbol_table);
    else if (is_jump_stmt(type))      analyze_jump_stmt_semantics(ast, symbol_table);
    else                              assert_semantics(0);
}

// declaration-semantics-analyzer
void analyze_global_declaration_list_semantics(Ast* ast, SymbolTable* symbol_table) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        analyze_global_declaration_semantics(ast_nth_child(ast, i), symbol_table);
    }
}

void analyze_global_declaration_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* ident = ast_nth_child(ast, 0);
    // Ast* init = NULL;
    SymbolStatus status = SYMBOL_DECL;

    switch(ast->type) {
        case AST_IDENT_DECL:
            assert_semantics(0);
            // TODO: global variables
            break;
        case AST_ARRAY_DECL:
            assert_semantics(0);
            // TODO: global arrays
            break;
        case AST_FUNC_DECL:
            status = SYMBOL_DECL;
            apply_inplace_function_declaration_conversion(ast);
            break;
        default:
            assert_semantics(0);
    }

    symbol_table_insert_copy(symbol_table, ident->value_ident, ident->ctype, status);
}

void analyze_local_declaration_list_semantics(Ast* ast, SymbolTable* symbol_table) {
    size_t i = 0, size = ast->children->size;
    for (i = 0; i < size; i++) {
        analyze_local_declaration_semantics(ast_nth_child(ast, i), symbol_table);
    }
}

void analyze_local_declaration_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* ident = ast_nth_child(ast, 0);
    Ast* init = NULL;
    SymbolStatus status = SYMBOL_DECL;

    switch(ast->type) {
        case AST_IDENT_DECL:
            status = SYMBOL_DEF;
            if (ast->children->size == 1) break;
            init = ast_nth_child(ast, 1);
            analyze_expr_semantics(init, symbol_table);
            assert_semantics(ctype_compatible(ident->ctype, init->ctype));
            break;
        case AST_ARRAY_DECL:
            status = SYMBOL_DEF;
            if (ast->children->size == 1) break;
            // TODO: initializer of arrays
            break;
        case AST_FUNC_DECL:
            status = SYMBOL_DECL;
            apply_inplace_function_declaration_conversion(ast);
            break;
        default:
            assert_semantics(0);
    }

    symbol_table_insert_copy(symbol_table, ident->value_ident, ident->ctype, status);
}

// external-declaration-semantics-analyzer
void analyze_external_declaration_semantics(Ast* ast, SymbolTable* symbol_table) {
    if (is_declaration_list(ast->type)) {
        analyze_global_declaration_list_semantics(ast, symbol_table);
    } else if (ast->type == AST_FUNC_DEF) {
        analyze_function_definition_semantics(ast, symbol_table);
    } else {
        assert_semantics(0);
    }
}

void analyze_function_definition_semantics(Ast* ast, SymbolTable* symbol_table) {
    Ast* func_decl = ast_nth_child(ast, 0);
    Ast* param_list = ast_nth_child(func_decl, 1);
    Ast* block = ast_nth_child(ast, 1);

    apply_inplace_function_declaration_conversion(func_decl);

    Ast* func_ident = ast_nth_child(func_decl, 0);
    symbol_table_insert_copy(symbol_table, func_ident->value_ident, func_ident->ctype, SYMBOL_DEF);

    SymbolTable* func_table = symbol_table_new(symbol_table);
    block->symbol_table = func_table;

    size_t i = 0, size = param_list->children->size;
    for (i = 0; i < size; i++) {
        Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
        symbol_table_insert_copy(func_table, param_ident->value_ident, param_ident->ctype, SYMBOL_DEF);
    }

    i = 0, size = block->children->size;
    for (i = 0; i < size; i++) {
        Ast* child = ast_nth_child(block, i);
        if (is_declaration_list(child->type)) {
            analyze_local_declaration_list_semantics(child, func_table);
        } else {
            analyze_stmt_semantics(child, func_table);
        }
    }
}

// assertion
void assert_semantics(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze semantics\n");
    exit(1);
}
