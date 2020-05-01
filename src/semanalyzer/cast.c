#include "cast.h"

#include <stdio.h>
#include <stdlib.h>
#include "../common/memory.h"


void apply_inplace_integer_promotion(Ast* ast) {
    if (
        ast->ctype == NULL ||
        ast->ctype->basic_ctype != CTYPE_CHAR
    )
    return;

    ast->ctype->basic_ctype = CTYPE_INT;
}

void apply_inplace_usual_arithmetic_conversion(Ast* ast) {
    apply_inplace_integer_promotion(ast_nth_child(ast, 0));
    apply_inplace_integer_promotion(ast_nth_child(ast, 1));
}

void apply_inplace_array_to_ptr_conversion(Ast* ast) {
    if (
        ast->ctype == NULL ||
        ast->ctype->basic_ctype != CTYPE_ARRAY
    )
        return;

    Ast* array = ast_new_ident(AST_IDENT, str_new(ast->value_ident));
    array->ctype = ctype_copy(ast->ctype);
    Ast* ptr = ast_new(AST_ARRAY_TO_PTR, 1, array);
    ptr->ctype = ctype_new_ptr(ctype_copy(array->ctype->array_of));
    ast_move(ast, ptr);
}

void revert_inplace_array_to_ptr_conversion(Ast* ast) {
    if (ast->type != AST_ARRAY_TO_PTR) return;

    Ast* array = ast_nth_child(ast, 0);
    ast_set_nth_child(ast, 0, NULL);
    ast_move(ast, array);
}

void apply_inplace_function_declaration_conversion(Ast* ast) {
    Ast* func_ident = ast_nth_child(ast, 0);
    if (func_ident->ctype->basic_ctype != CTYPE_FUNC) return;

    Ast* param_list = ast_nth_child(ast, 1);
    size_t i = 0, size = param_list->children->size;
    for (i = 0; i < size; i++) {
        CType* param_ctype = (CType*)vector_at(func_ident->ctype->func->param_list, i);
        CType* param_ident_ctype = ast_nth_child(ast_nth_child(param_list, i), 0)->ctype;
        switch (param_ctype->basic_ctype) {
            case CTYPE_CHAR:
            case CTYPE_INT:
            case CTYPE_PTR:
                break;
            case CTYPE_ARRAY: {
                CType* ctype_ptr = NULL;
                ctype_ptr = ctype_new_ptr(ctype_copy(param_ctype->array_of));
                ctype_move(param_ctype, ctype_ptr);
                ctype_ptr = ctype_new_ptr(ctype_copy(param_ident_ctype->array_of));
                ctype_move(param_ident_ctype, ctype_ptr);
                break;
            }
            case CTYPE_FUNC:
                fprintf(stderr, "Error: function as a parameter is not supported\n");
                // TODO: function as a param
                exit(0);
                break;
        }
    }
}
