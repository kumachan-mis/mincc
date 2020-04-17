#include "cast.h"

#include <stdio.h>
#include <stdlib.h>


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

    Ast* array = ast_new_ident(AST_IDENT, ast->value_ident);
    array->ctype = ast->ctype;
    Ast* ptr = ast_new(AST_ARRAY_TO_PTR, 1, array);
    ptr->ctype = ctype_new_ptr(ctype_copy(array->ctype->array_of));

    *ast = *ptr;
    free(ptr);
}

void apply_inplace_function_declaration_conversion(Ast* ast) {
    if (ast->type != AST_FUNC_DECL) return;

    Ast* func_ident = ast_nth_child(ast, 0);
    CType* func_ctype = func_ident->ctype;

    Ast* param_list = ast_nth_child(ast, 1);
    size_t i = 0, size = param_list->children->size;
    for (i = 0; i < size; i++) {
        Ast* param_decl =  ast_nth_child(param_list, i);
        Ast* param_ident = ast_nth_child(param_decl, 0);

        CType* param_ctype = (CType*)vector_at(func_ctype->func->param_types, i);
        CType* param_ident_ctype = param_ident->ctype;
        switch (param_decl->type) {
            case AST_IDENT_DECL:
                break;
            case AST_ARRAY_DECL: {
                CType* ctype_ptr = NULL;
                ctype_ptr = ctype_new_ptr(param_ctype->array_of);
                *param_ctype = *ctype_ptr;
                free(ctype_ptr);
                ctype_ptr = ctype_new_ptr(param_ident_ctype->array_of);
                *param_ident_ctype = *ctype_ptr;
                free(ctype_ptr);
                break;
            }
            case AST_FUNC_DECL:
                fprintf(stderr, "Error: function as a parameter is not supported\n");
                // TODO: function as a param
                exit(0);
                break;
            default:
                fprintf(stderr, "Error: cannot cast unknown type\n");
                exit(0);
        }
        param_decl->type = AST_IDENT_DECL;
    }
}
