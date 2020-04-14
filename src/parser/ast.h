#ifndef _AST_H_
#define _AST_H_


#include "symboltable.h"
#include "../common/vector.h"


typedef enum {
    // primary-expression
    AST_IDENT,
    AST_IMM_INT,
    // postfix-expression
    AST_FUNC_CALL,
    // argument-expression-list
    AST_ARG_LIST,
    // unary-expression
    AST_ADDR,
    AST_DEREF,
    AST_POSI,
    AST_NEGA,
    AST_NOT,
    AST_LNOT,
    // cast-expression
    AST_ARRAY_TO_PTR,
    // multiplicative-expression
    AST_MUL,
    AST_DIV,
    AST_MOD,
    // additive-expression
    AST_ADD,
    AST_SUB,
    // shift-expression
    AST_LSHIFT,
    AST_RSHIFT,
    // relational-expression
    AST_LT,
    AST_GT,
    AST_LEQ,
    AST_GEQ,
    // equality-expression
    AST_EQ,
    AST_NEQ,
    // bitwise-expression
    AST_AND,
    AST_XOR,
    AST_OR,
    // logical-expression
    AST_LAND,
    AST_LOR,
    // assignment-expression
    AST_ASSIGN,
    // null-expression
    AST_NULL,

    // compound-statement
    AST_COMP_STMT,
    // expression-statement
    AST_EXPR_STMT,
    // selection-statement
    AST_IF_STMT,
    // iteration-statement
    AST_WHILE_STMT,
    AST_DOWHILE_STMT,
    AST_FOR_STMT,
    // jump-statement
    AST_RETURN_STMT,

    // declaration
    AST_DECL_LIST,
    AST_IDENT_DECL,
    AST_ARRAY_DECL,
    AST_FUNC_DECL,
    AST_PARAM_LIST,

    // external-declaration
    AST_FUNC_DEF,
} AstType;

typedef struct {
    AstType type;
    CType* ctype;
    union {
        int value_int;
        char* value_ident;
        SymbolTable* symbol_table;
    };
    Vector* children;
} Ast;

typedef struct {
    Vector* inner_vector;
    int pos;
    SymbolTable* symbol_table;
} AstList;


// astlist
Ast* astlist_top(AstList* astlist);
void astlist_pop(AstList* astlist);
void astlist_delete(AstList* astlist);
Ast* ast_nth_child(Ast* ast, size_t n);

// ast
AstList* astlist_new();
Ast* ast_new(AstType type, size_t num_children, ...);
Ast* ast_new_int(AstType type, int value_int);
Ast* ast_new_ident(AstType type, char* value_ident);
void ast_append_child(Ast* ast, Ast* child);
void ast_delete(Ast* ast);

// expression-ast-classifier
int is_primary_expr(AstType type);
int is_postfix_expr(AstType type);
int is_unary_expr(AstType type);
int is_cast_expr(AstType type);
int is_multiplicative_expr(AstType type);
int is_additive_expr(AstType type);
int is_shift_expr(AstType type);
int is_equality_expr(AstType type);
int is_relational_expr(AstType type);
int is_bitwise_expr(AstType type);
int is_logical_expr(AstType type);
int is_assignment_expr(AstType type);
int is_null_expr(AstType type);

// statement-ast-classifier
int is_compound_stmt(AstType type);
int is_expr_stmt(AstType type);
int is_selection_stmt(AstType type);
int is_iteration_stmt(AstType type);
int is_jump_stmt(AstType type);

// declaration-ast-classifier
int is_declaration_list(AstType type);
int is_declaration(AstType type);


#endif  // _AST_H_
