#ifndef _PARSER_H_
#define _PARSER_H_


#include "lex.h"
#include "vector.h"


typedef enum {
    // primary-expression
    AST_VAR,
    AST_INT,

    // postfix-expression
    AST_CALL,
    // argument-expression-list
    AST_ARG_LIST,

    // unary-expression:
    AST_POSI,
    AST_NEGA,
    AST_NOT,
    AST_LNOT,

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

    // null-expression
    AST_NULL,

    // assignment-expression
    AST_ASSIGN,

    // expression-statement
    AST_EXPR_STMT,

    // selection-statement
    AST_IF_STMT,

    // iteration-statement
    AST_WHILE_STMT,

    // jump-statement
    AST_RETURN_STMT,
} AstType;


typedef struct {
    AstType type;
    union {
        int value_int;
        char* value_ident;
    };
    Vector* children;
} Ast;

typedef struct {
    Vector* inner_vector;
    int pos;
} AstList;

AstList* parse(TokenList* tokenlist);

Ast* astlist_top(AstList* astlist);
void astlist_pop(AstList* astlist);
void astlist_delete(AstList* astlist);
Ast* ast_nth_child(Ast* ast, size_t n);

int is_primary_expr(AstType type);
int is_postfix_expr(AstType type);
int is_unary_expr(AstType type);
int is_multiplicative_expr(AstType type);
int is_additive_expr(AstType type);
int is_shift_expr(AstType type);
int is_equality_expr(AstType type);
int is_relational_expr(AstType type);
int is_bitwise_expr(AstType type);
int is_logical_expr(AstType type);
int is_assignment_expr(AstType type);
int is_null_expr(AstType type);
int is_expr_stmt(AstType type);
int is_selection_stmt(AstType type);
int is_iteration_stmt(AstType type);
int is_jump_stmt(AstType type);


#endif  // _PARSER_H_
