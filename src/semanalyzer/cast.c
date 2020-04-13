#include "cast.h"


Ast* cast_array_to_ptr(Ast* ast) {
    if (
        ast->ctype == NULL ||
        ast->ctype->basic_ctype != CTYPE_ARRAY
    )
        return ast;

    return ast_new(AST_ARRAY_TO_PTR, 1, ast);
}

void cast_inline_array_to_ptr(Ast* ast) {
    if (
        ast->ctype == NULL ||
        ast->ctype->basic_ctype != CTYPE_ARRAY
    )
        return;

    Ast* array = ast_new_ident(AST_IDENT, ast->value_ident);
    array->ctype = ast->ctype;

    ast->type = AST_ARRAY_TO_PTR;
    ast->ctype = ctype_new_ptr(ctype_copy(array->ctype->array_of));
    vector_delete(ast->children);
    ast->children = vector_new();
    ast_append_child(ast, array);
}
