#include "parser.h"

#include <stdlib.h>
#include <stdarg.h>
#include "lex.h"
#include "memory.h"

// expression-parser
Ast* parse_primary_expr(TokenList* tokenlist);
Ast* parse_postfix_expr(TokenList* tokenlist);
Ast* parse_arg_expr_list(TokenList* tokenlist);
Ast* parse_unary_expr(TokenList* tokenlist);
Ast* parse_multiplicative_expr(TokenList* tokenlist);
Ast* parse_additive_expr(TokenList* tokenlist);
Ast* parse_shift_expr(TokenList* tokenlist);
Ast* parse_relational_expr(TokenList* tokenlist);
Ast* parse_equality_expr(TokenList* tokenlist);
Ast* parse_and_expr(TokenList* tokenlist);
Ast* parse_xor_expr(TokenList* tokenlist);
Ast* parse_or_expr(TokenList* tokenlist);
Ast* parse_logical_and_expr(TokenList* tokenlist);
Ast* parse_logical_or_expr(TokenList* tokenlist);
Ast* parse_assignment_expr(TokenList* tokenlist);
Ast* parse_expr(TokenList* tokenlist);

// statement-parser
Ast* parse_expr_stmt(TokenList* tokenlist);
Ast* parse_selection_stmt(TokenList* tokenlist);
Ast* parse_jump_stmt(TokenList* tokenlist);
Ast* parse_stmt(TokenList* tokenlist);

// ast
AstList* astlist_new();
Ast* ast_new(AstType type, size_t num_children, ...);
void ast_append_child(Ast* ast, Ast* child);
void ast_delete(Ast* ast);

// assertion
void assert_syntax(int condition);


AstList* parse(TokenList* tokenlist) {
    AstList* astlist = astlist_new();
    Vector* inner_vector = astlist->inner_vector;
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_EOF) break;
        Ast* ast = parse_stmt(tokenlist);
        vector_push_back(inner_vector, ast);
    }
    return astlist;
}

// expression-parser
Ast* parse_primary_expr(TokenList* tokenlist) {
    Ast* ast = NULL;
    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);

    switch (token->type) {
        case TOKEN_INT_CONST:
            ast = ast_new(AST_INT, 0);
            ast->value_int = token->value_int;
            break;
        case TOKEN_IDENT:
            ast = ast_new(AST_VAR, 0);
            ast->value_ident = str_new(token->value_ident);
            break;
        case TOKEN_LPAREN:
            ast = parse_expr(tokenlist);
            token = tokenlist_top(tokenlist);
            tokenlist_pop(tokenlist);
            assert_syntax(token->type == TOKEN_RPAREN);
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_postfix_expr(TokenList* tokenlist) {
    Ast* ast = parse_primary_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_LPAREN:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_CALL, 2, ast, parse_arg_expr_list(tokenlist));
                token = tokenlist_top(tokenlist);
                tokenlist_pop(tokenlist);
                assert_syntax(token->type == TOKEN_RPAREN);
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_arg_expr_list(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_ARG_LIST, 0);
    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_RPAREN) return ast;
    
    ast_append_child(ast, parse_assignment_expr(tokenlist));
    while (1) {
        token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RPAREN) return ast;
    
        tokenlist_pop(tokenlist);
        assert_syntax(token->type == TOKEN_COMMA);
        ast_append_child(ast, parse_assignment_expr(tokenlist));
    }
}

Ast* parse_unary_expr(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_PLUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_POSI, 1, parse_unary_expr(tokenlist));
            break;
        case TOKEN_MINUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_NEGA, 1, parse_unary_expr(tokenlist));
            break;
        case TOKEN_TILDER:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_NOT,  1, parse_unary_expr(tokenlist));
            break;
        case TOKEN_EXCL:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_LNOT, 1, parse_unary_expr(tokenlist));
            break;
        default:
            ast = parse_postfix_expr(tokenlist);
            break;
    }
    return ast;
}

Ast* parse_multiplicative_expr(TokenList* tokenlist) {
    Ast* ast = parse_unary_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_ASTERISK:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_MUL, 2, ast, parse_unary_expr(tokenlist));
                break;
            case TOKEN_SLASH:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_DIV, 2, ast, parse_unary_expr(tokenlist));
                break;
            case TOKEN_PERCENT:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_MOD, 2, ast, parse_unary_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_additive_expr(TokenList* tokenlist) {
    Ast* ast = parse_multiplicative_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_PLUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_ADD, 2, ast, parse_multiplicative_expr(tokenlist));
                break;
            case TOKEN_MINUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_SUB, 2, ast, parse_multiplicative_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_shift_expr(TokenList* tokenlist) {
    Ast* ast = parse_additive_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_LANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LSHIFT, 2, ast, parse_additive_expr(tokenlist));
                break;
            case TOKEN_DBL_RANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_RSHIFT, 2, ast, parse_additive_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_relational_expr(TokenList* tokenlist) {
    Ast* ast = parse_shift_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_LANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LT, 2, ast, parse_shift_expr(tokenlist));
                break;
            case TOKEN_RANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_GT, 2, ast, parse_shift_expr(tokenlist));
                break;
            case TOKEN_LANGLE_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LEQ, 2, ast, parse_shift_expr(tokenlist));
                break;
            case TOKEN_RANGLE_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_GEQ, 2, ast, parse_shift_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_equality_expr(TokenList* tokenlist) {
    Ast* ast = parse_relational_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_EQ, 2, ast, parse_relational_expr(tokenlist));
                break;
            case TOKEN_EXCL_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_NEQ, 2, ast, parse_relational_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_and_expr(TokenList* tokenlist) {
    Ast* ast = parse_equality_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_AND:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_AND, 2, ast, parse_equality_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_xor_expr(TokenList* tokenlist) {
    Ast* ast = parse_and_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_HAT:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_XOR, 2, ast, parse_and_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_or_expr(TokenList* tokenlist) {
    Ast* ast = parse_xor_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_BAR:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_OR, 2, ast, parse_xor_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_logical_and_expr(TokenList* tokenlist) {
    Ast* ast = parse_or_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_AND:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LAND, 2, ast, parse_or_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_logical_or_expr(TokenList* tokenlist) {
    Ast* ast = parse_logical_and_expr(tokenlist);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_BAR:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LOR, 2, ast, parse_logical_and_expr(tokenlist));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_assignment_expr(TokenList* tokenlist) {
    size_t pos_memo = tokenlist->pos;
    Ast* ast = parse_unary_expr(tokenlist);

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_EQ:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_ASSIGN, 2, ast, parse_assignment_expr(tokenlist));
            break;
        default:
            tokenlist->pos = pos_memo;
            ast = parse_logical_or_expr(tokenlist);
            break;
    }
    return ast;
}

Ast* parse_expr(TokenList* tokenlist) {
    return parse_assignment_expr(tokenlist);
}

// statement-parser
Ast* parse_expr_stmt(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_SEMICOLON) {
        ast = ast_new(AST_NULL_STMT, 0);
        tokenlist_pop(tokenlist);
    } else {
        ast = ast_new(AST_EXPR_STMT, 0);
        ast_append_child(ast, parse_expr(tokenlist));
    
        token = tokenlist_top(tokenlist);
        tokenlist_pop(tokenlist);
        assert_syntax(token->type == TOKEN_SEMICOLON);
    }
    return ast;
}

Ast* parse_selection_stmt(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_IF:
            ast = ast_new(AST_IF_STMT, 0);
    
            token = tokenlist_top(tokenlist);
            tokenlist_pop(tokenlist);
            assert_syntax(token->type == TOKEN_LPAREN);

            ast_append_child(ast, parse_expr(tokenlist));

            token = tokenlist_top(tokenlist);
            tokenlist_pop(tokenlist);
            assert_syntax(token->type == TOKEN_RPAREN);

            ast_append_child(ast, parse_stmt(tokenlist));

            token = tokenlist_top(tokenlist);
            if (token->type == TOKEN_ELSE) {
                tokenlist_pop(tokenlist);
                ast_append_child(ast, parse_stmt(tokenlist));
            }
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_jump_stmt(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_RETURN:
            ast = ast_new(AST_RETURN_STMT, 0);
            ast_append_child(ast, parse_expr(tokenlist));
    
            token = tokenlist_top(tokenlist);
            tokenlist_pop(tokenlist);
            assert_syntax(token->type == TOKEN_SEMICOLON);
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_stmt(TokenList* tokenlist) {
    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_IF) {
        return parse_selection_stmt(tokenlist);
    } else if (token->type == TOKEN_RETURN) {
        return parse_jump_stmt(tokenlist);
    } else {
        return parse_expr_stmt(tokenlist);
    }
}

// ast
AstList* astlist_new() {
    AstList* astlist = (AstList*)safe_malloc(sizeof(AstList));
    astlist->inner_vector = vector_new();
    astlist->pos = 0;
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
    free(astlist);
}

Ast* ast_new(AstType type, size_t num_children, ...) {
    Ast* ast = (Ast*)safe_malloc(sizeof(Ast));
    ast->type = type;

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

void ast_append_child(Ast* ast, Ast* child) {
    return vector_push_back(ast->children, child);
}

Ast* ast_nth_child(Ast* ast, size_t n) {
    return (Ast*)vector_at(ast->children, n);
}

void ast_delete(Ast* ast) {
    Vector* children = ast->children;
    size_t i = 0, size = children->size;
    for (i = 0; i < size; i++) {
        ast_delete(vector_at(children, i));
        children->data[i] = NULL;
    }
    free(children);

    if (ast->type == AST_VAR) {
        free(ast->value_ident);
    }
    free(ast);
}

// asttype-classifier
int is_primary_expr(AstType type) {
    return type == AST_INT || type == AST_VAR;
}

int is_postfix_expr(AstType type) {
    return type == AST_CALL;
}

int is_unary_expr(AstType type) {
    return type == AST_POSI || type == AST_NEGA ||
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

int is_expr_stmt(AstType type) {
    return type == AST_EXPR_STMT || type == AST_NULL_STMT;
}

int is_selection_stmt(AstType type) {
    return type == AST_IF_STMT;
}

int is_jump_stmt(AstType type) {
    return type == AST_RETURN_STMT;
}

void assert_syntax(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to parse input\n");
    exit(1);
}
