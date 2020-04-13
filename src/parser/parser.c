#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include "../common/memory.h"


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
Ast* parse_compound_stmt(TokenList* tokenlist);
Ast* parse_expr_stmt(TokenList* tokenlist);
Ast* parse_selection_stmt(TokenList* tokenlist);
Ast* parse_iteration_stmt(TokenList* tokenlist);
Ast* parse_jump_stmt(TokenList* tokenlist);
Ast* parse_stmt(TokenList* tokenlist);

// declaration-parser
Ast* parse_declaration(TokenList* tokenlist);
CType* parse_type_specifier(TokenList* tokenlist);
Ast* parse_init_declarator(TokenList* tokenlist, CType* basic_ctype);
Ast* parse_declarator(TokenList* tokenlist, CType* basic_ctype);
Ast* parse_direct_declarator(TokenList* tokenlist, CType* ctype);
Ast* parse_param_list(TokenList* tokenlist);
Ast* parse_param_declaration(TokenList* tokenlist);

// external-definition-parser
Ast* parse_function_definition(TokenList* tokenlist);

// assertion
Token* assert_and_top_token(TokenList* tokenlist, TokenType expected_type);
void assert_and_pop_token(TokenList* tokenlist, TokenType expected_type);
void assert_syntax(int condition);


AstList* parse(TokenList* tokenlist) {
    AstList* astlist = astlist_new();
    Vector* inner_vector = astlist->inner_vector;
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_EOF) break;
        Ast* ast = parse_function_definition(tokenlist);
        vector_push_back(inner_vector, ast);
    }
    tokenlist->pos = 0;
    return astlist;
}

// expression-parser
Ast* parse_primary_expr(TokenList* tokenlist) {
    Ast* ast = NULL;
    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);

    switch (token->type) {
        case TOKEN_IMM_INT:
            ast = ast_new_int(AST_IMM_INT, token->value_int);
            break;
        case TOKEN_IDENT:
            ast = ast_new_ident(AST_IDENT, str_new(token->value_ident));
            break;
        case TOKEN_LPAREN:
            ast = parse_expr(tokenlist);
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
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
            case TOKEN_LBRACKET:
                tokenlist_pop(tokenlist);
                ast = ast_new(
                    AST_DEREF, 1,
                    ast_new(AST_ADD, 2, ast, parse_assignment_expr(tokenlist))
                );
                assert_and_pop_token(tokenlist, TOKEN_RBRACKET);
                break;
            case TOKEN_LPAREN:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_FUNC_CALL, 2, ast, parse_arg_expr_list(tokenlist));
                assert_and_pop_token(tokenlist, TOKEN_RPAREN);
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
        assert_and_pop_token(tokenlist, TOKEN_COMMA);
        ast_append_child(ast, parse_assignment_expr(tokenlist));
    }
}

Ast* parse_unary_expr(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_AND:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_ADDR, 1, parse_unary_expr(tokenlist));
            break;
        case TOKEN_ASTERISK:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_DEREF, 1, parse_unary_expr(tokenlist));
            break;
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
Ast* parse_compound_stmt(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_COMP_STMT, 0);

    assert_and_pop_token(tokenlist, TOKEN_LBRACE);
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RBRACE) {
            tokenlist_pop(tokenlist);
            break;
        }
        if (token->type == TOKEN_INT) ast_append_child(ast, parse_declaration(tokenlist));
        else                          ast_append_child(ast, parse_stmt(tokenlist));
    }
    return ast;
}

Ast* parse_expr_stmt(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_EXPR_STMT, 0);

    Token* token = tokenlist_top(tokenlist);
    ast_append_child(
        ast,
        token->type == TOKEN_SEMICOLON
        ? ast_new(AST_NULL, 0)
        : parse_expr(tokenlist)
    );
    assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
    return ast;
}

Ast* parse_selection_stmt(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_IF:
            ast = ast_new(AST_IF_STMT, 0);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
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

Ast* parse_iteration_stmt(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_WHILE:
            ast = ast_new(AST_WHILE_STMT, 0);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            ast_append_child(ast, parse_stmt(tokenlist));
            break;
        case TOKEN_DO:
            ast = ast_new(AST_DOWHILE_STMT, 0);
            ast_append_child(ast, parse_stmt(tokenlist));
            assert_and_pop_token(tokenlist, TOKEN_WHILE);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            break;
        case TOKEN_FOR:
            ast = ast_new(AST_FOR_STMT, 0);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            token = tokenlist_top(tokenlist);
            ast_append_child(
                ast,
                token->type == TOKEN_SEMICOLON
                ? ast_new(AST_NULL, 0)
                : parse_expr(tokenlist)
            );
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            token = tokenlist_top(tokenlist);
            ast_append_child(
                ast,
                token->type == TOKEN_SEMICOLON
                ? ast_new(AST_NULL, 0)
                : parse_expr(tokenlist)
            );
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            token = tokenlist_top(tokenlist);
            ast_append_child(
                ast,
                token->type == TOKEN_RPAREN
                ? ast_new(AST_NULL, 0)
                : parse_expr(tokenlist)
            );
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            ast_append_child(ast, parse_stmt(tokenlist));
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
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_stmt(TokenList* tokenlist) {
    Token* token = tokenlist_top(tokenlist);
    TokenType type = token->type;
    if (type == TOKEN_LBRACE) {
        return parse_compound_stmt(tokenlist);
    } else if (type == TOKEN_IF) {
        return parse_selection_stmt(tokenlist);
    } else if (
        type == TOKEN_WHILE ||
        type == TOKEN_DO ||
        type == TOKEN_FOR
    ) {
        return parse_iteration_stmt(tokenlist);
    } else if (type == TOKEN_RETURN) {
        return parse_jump_stmt(tokenlist);
    } else {
        return parse_expr_stmt(tokenlist);
    }
}

// declaration-parser
Ast* parse_declaration(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_DECL_LIST, 0);
    CType* basic_ctype = parse_type_specifier(tokenlist);

    Ast* child = parse_init_declarator(tokenlist, ctype_copy(basic_ctype));
    ast_append_child(ast, child);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_SEMICOLON) break;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);

        child = parse_init_declarator(tokenlist, ctype_copy(basic_ctype));
        ast_append_child(ast, child);
    }

    ctype_delete(basic_ctype);
    return ast;
}

CType* parse_type_specifier(TokenList* tokenlist) {
    CType* ctype = NULL;

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_INT:
            tokenlist_pop(tokenlist);
            ctype = ctype_new_int();
            break;
        default:
            assert_syntax(0);
    }
    return ctype;
}

Ast* parse_init_declarator(TokenList* tokenlist, CType* basic_ctype) {
    Ast* ast = parse_declarator(tokenlist, basic_ctype);

    Token* token = tokenlist_top(tokenlist);
    switch (ast->type) {
        case AST_IDENT_DECL:
            if (token->type != TOKEN_EQ) break;
            tokenlist_pop(tokenlist);
            ast_append_child(ast, parse_assignment_expr(tokenlist));
            break;
        case AST_ARRAY_DECL:
            if (token->type != TOKEN_EQ) break;
            // TODO: initializer of arrays
            break;
        case AST_FUNC_DECL:
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_declarator(TokenList* tokenlist, CType* basic_ctype) {
    CType* ctype = basic_ctype;
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type != TOKEN_ASTERISK) break;
        tokenlist_pop(tokenlist);
        ctype = ctype_new_ptr(ctype);
    }
    Ast* ast = parse_direct_declarator(tokenlist, ctype);
    return ast;
}

Ast* parse_direct_declarator(TokenList* tokenlist, CType* ctype) {
    Ast* ast = NULL;

    Token* token_ident = assert_and_top_token(tokenlist, TOKEN_IDENT);
    tokenlist_pop(tokenlist);
    Ast* ident = ast_new_ident(AST_IDENT, str_new(token_ident->value_ident));

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_LBRACKET:
            tokenlist_pop(tokenlist);
    
            token = assert_and_top_token(tokenlist, TOKEN_IMM_INT);
            tokenlist_pop(tokenlist);
            ident->ctype = ctype_new_array(ctype, token->value_int);
            // TODO: assignment-expr for array length

            ast = ast_new(AST_ARRAY_DECL, 1, ident);
            assert_and_pop_token(tokenlist, TOKEN_RBRACKET);
            break;
        case TOKEN_LPAREN:
            tokenlist_pop(tokenlist);
            ident->ctype = ctype;
            ast = ast_new(AST_FUNC_DECL, 2, ident, parse_param_list(tokenlist));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            break;
        default:
            ident = ast_new_ident(AST_IDENT, str_new(token_ident->value_ident));
            ident->ctype = ctype;
            ast = ast_new(AST_IDENT_DECL, 1, ident);
            break;
    }
    // TODO: loop for complex declatations
    return ast;
}

Ast* parse_param_list(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_PARAM_LIST, 0);
    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_RPAREN) return ast;

    ast_append_child(ast, parse_param_declaration(tokenlist));
    while (1) {
        token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RPAREN) break;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);
        ast_append_child(ast, parse_param_declaration(tokenlist));
    }
    return ast;
}

Ast* parse_param_declaration(TokenList* tokenlist) {
    CType* basic_ctype = parse_type_specifier(tokenlist);
    Ast* ast = parse_declarator(tokenlist, basic_ctype);
    return ast;
}

// external-definition-parser
Ast* parse_function_definition(TokenList* tokenlist) {
    CType* ctype = parse_type_specifier(tokenlist);
    Ast* decl = parse_declarator(tokenlist, ctype);
    assert_syntax(decl->type == AST_FUNC_DECL);
    Ast* ast = ast_new(AST_FUNC_DEF, 2, decl, parse_compound_stmt(tokenlist));   
    return ast;
}

// assertion
Token* assert_and_top_token(TokenList* tokenlist, TokenType expected_type) {
    Token* token = tokenlist_top(tokenlist);
    assert_syntax(token->type == expected_type);
    return token;
}

void assert_and_pop_token(TokenList* tokenlist, TokenType expected_type) {
    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    assert_syntax(token->type == expected_type);
}

void assert_syntax(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to parse input\n");
    exit(1);
}
