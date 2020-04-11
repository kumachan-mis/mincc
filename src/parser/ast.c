#include "ast.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include "type.h"
#include "../common/memory.h"


// astlist
AstList* astlist_new() {
    AstList* astlist = (AstList*)safe_malloc(sizeof(AstList));
    astlist->inner_vector = vector_new();
    astlist->pos = 0;
    astlist->symbol_table = NULL;
    return astlist;
}

Ast* astlist_top(AstList* astlist) {
    return (Ast*)vector_at(astlist->inner_vector, astlist->pos);
}

void astlist_pop(AstList* astlist) {
    astlist->pos++;
}

void astlist_delete(AstList* astlist) {
    Vector* inner_vector = astlist->inner_vector;
    size_t i = 0, size = inner_vector->size;
    for (i = 0; i < size; i++) {
        ast_delete(vector_at(inner_vector, i));
        inner_vector->data[i] = NULL;
    }
    vector_delete(inner_vector);
    symbol_table_delete(astlist->symbol_table);
    free(astlist);
}

// ast
Ast* ast_new(AstType type, size_t num_children, ...) {
    Ast* ast = (Ast*)safe_malloc(sizeof(Ast));
    ast->type = type;
    ast->ctype = NULL;

    ast->children = vector_new();
    Vector* children = ast->children;
    vector_reserve(children, num_children);
    va_list childlist;
    va_start(childlist, num_children);
    size_t i;
    for (i = 0; i < num_children; i++) {
        vector_push_back(children, va_arg(childlist, Ast*));

    }
    va_end(childlist);

    return ast;
}

Ast* ast_new_int(AstType type, int value_int) {
    Ast* ast = ast_new(type, 0);
    ast->value_int = value_int;
    ast->ctype = NULL;
    return ast;
}

Ast* ast_new_ident(AstType type, char* value_ident) {
    Ast* ast = ast_new(type, 0);
    ast->value_ident = value_ident;
    ast->ctype = NULL;
    return ast;
}

void ast_append_child(Ast* ast, Ast* child) {
    return vector_push_back(ast->children, child);
}

Ast* ast_nth_child(Ast* ast, size_t n) {
    return (Ast*)vector_at(ast->children, n);
}

void ast_delete(Ast* ast) {
    ctype_delete(ast->ctype);
    switch (ast->type) {
        case AST_IDENT:
            free(ast->value_ident);
            break;
        case AST_COMP_STMT:
            symbol_table_delete(ast->symbol_table);
            break;
        default:
            break;
    }

    Vector* children = ast->children;
    size_t i = 0, size = children->size;
    for (i = 0; i < size; i++) {
        ast_delete(vector_at(children, i));
        children->data[i] = NULL;
    }
    free(children);
    free(ast);
}

// expression-ast-classifier
int is_primary_expr(AstType type) {
    return type == AST_IMM_INT || type == AST_IDENT;
}

int is_postfix_expr(AstType type) {
    return type == AST_FUNC_CALL;
}

int is_unary_expr(AstType type) {
    return type == AST_ADDR || type == AST_DEREF ||
           type == AST_POSI || type == AST_NEGA  ||
           type == AST_NOT  || type == AST_LNOT;
}

int is_multiplicative_expr(AstType type) {
    return type == AST_MUL || type == AST_DIV || type == AST_MOD;
}

int is_additive_expr(AstType type) {
    return type == AST_ADD || type == AST_SUB;
}

int is_shift_expr(AstType type) {
    return type == AST_LSHIFT || type == AST_RSHIFT;
}

int is_equality_expr(AstType type) {
    return type == AST_EQ  || type == AST_NEQ;
}

int is_relational_expr(AstType type) {
    return type == AST_LT  || type == AST_GT  ||
           type == AST_LEQ || type == AST_GEQ;
}

int is_bitwise_expr(AstType type) {
    return type == AST_AND || type == AST_XOR || type == AST_OR;
}

int is_logical_expr(AstType type) {
    return type == AST_LAND || type == AST_LOR;
}

int is_assignment_expr(AstType type) {
    return type == AST_ASSIGN;
}

int is_null_expr(AstType type) {
    return type == AST_NULL;
}

// statement-ast-classifier
int is_compound_stmt(AstType type) {
    return type == AST_COMP_STMT;
}

int is_expr_stmt(AstType type) {
    return type == AST_EXPR_STMT;
}

int is_selection_stmt(AstType type) {
    return type == AST_IF_STMT;
}

int is_iteration_stmt(AstType type) {
    return type == AST_WHILE_STMT || type == AST_DOWHILE_STMT ||
           type == AST_FOR_STMT;
}

int is_jump_stmt(AstType type) {
    return type == AST_RETURN_STMT;
}

// declaration-ast-classifier
int is_declaration_list(AstType type) {
    return type == AST_DECL_LIST;
}

int is_declaration(AstType type) {
    return type == AST_IDENT_DECL || type == AST_FUNC_DECL;
}