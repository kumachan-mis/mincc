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
Ast* parse_array_len_list(TokenList* tokenlist);
Ast* parse_param_list(TokenList* tokenlist);
Ast* parse_param_declaration(TokenList* tokenlist);
Ast* parse_initializer(TokenList* tokenlist);
Ast* parse_initializer_list(TokenList* tokenlist);

// external-declaration-parser
Ast* parse_external_declaration(TokenList* tokenlist);

// assertion
Token* assert_and_top_token(TokenList* tokenlist, TokenType expected_type);
void assert_and_pop_token(TokenList* tokenlist, TokenType expected_type);
void assert_syntax(int condition);


AstList* parse(TokenList* tokenlist) {
    AstList* astlist = astlist_new();
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_EOF) break;
        Ast* ast = parse_external_declaration(tokenlist);
        astlist_append(astlist, ast);
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
        case TOKEN_IMM_STR:
            ast = ast_new_str(AST_IMM_STR, str_new(token->value_str));
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
            case TOKEN_DBL_PLUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_POST_INCR, 1, ast);
                break;
            case TOKEN_DBL_MINUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_POST_DECR, 1, ast);
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
        case TOKEN_DBL_PLUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_PRE_INCR, 1, parse_unary_expr(tokenlist));
            break;
        case TOKEN_DBL_MINUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_PRE_DECR, 1, parse_unary_expr(tokenlist));
            break;
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
    int op_type = -1;
    switch (token->type) {
        case TOKEN_EQ:
            op_type = AST_NULL;
            break;
        case TOKEN_ASTERISK_EQ:
            op_type = AST_MUL;
            break;
        case TOKEN_SLASH_EQ:
            op_type = AST_DIV;
            break;
        case TOKEN_PERCENT_EQ:
            op_type = AST_MOD;
            break;
        case TOKEN_PLUS_EQ:
            op_type = AST_ADD;
            break;
        case TOKEN_MINUS_EQ:
            op_type = AST_SUB;
            break;
        case TOKEN_DBL_LANGLE_EQ:
            op_type = AST_LSHIFT;
            break;
        case TOKEN_DBL_RANGLE_EQ:
            op_type = AST_RSHIFT;
            break;
        case TOKEN_AND_EQ:
            op_type = AST_AND;
            break;
        case TOKEN_HAT_EQ:
            op_type = AST_XOR;
            break;
        case TOKEN_BAR_EQ:
            op_type = AST_OR;
            break;
        default:
            op_type = -1;
            break;
    }

    if (op_type < 0) {
        tokenlist->pos = pos_memo;
        ast_delete(ast);
        ast = parse_logical_or_expr(tokenlist);
    } else if (op_type == AST_NULL) {
        tokenlist_pop(tokenlist);
        ast = ast_new(AST_ASSIGN, 2, ast, parse_assignment_expr(tokenlist));
    } else {
        tokenlist_pop(tokenlist);
        Ast* op_ast = ast_new(op_type, 2, ast_copy(ast), parse_assignment_expr(tokenlist));
        ast = ast_new(AST_ASSIGN, 2, ast, op_ast);
    }
    return ast;
}

Ast* parse_expr(TokenList* tokenlist) {
    return parse_assignment_expr(tokenlist);
}

// statement-parser
Ast* parse_compound_stmt(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_COMP_STMT, 0);
    ast->local_table = NULL;

    assert_and_pop_token(tokenlist, TOKEN_LBRACE);
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RBRACE) {
            tokenlist_pop(tokenlist);
            break;
        }
        if (token->type == TOKEN_CHAR || token->type == TOKEN_INT) {
            ast_append_child(ast, parse_declaration(tokenlist));
        } else {
            ast_append_child(ast, parse_stmt(tokenlist));
        }
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
        if (token->type == TOKEN_SEMICOLON) {
            tokenlist_pop(tokenlist);
            break;
        }
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
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_INT:
            ctype = ctype_new_int();
            break;
        case TOKEN_CHAR:
            ctype = ctype_new_char();
            break;
        default:
            assert_syntax(0);
    }
    return ctype;
}

Ast* parse_init_declarator(TokenList* tokenlist, CType* basic_ctype) {
    Ast* ast = parse_declarator(tokenlist, basic_ctype);
    Ast* ident = ast_nth_child(ast, 0);

    Token* token = tokenlist_top(tokenlist);
    switch (ident->ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
        case CTYPE_PTR:
        case CTYPE_ARRAY:
            if (token->type != TOKEN_EQ) break;
            tokenlist_pop(tokenlist);
            ast_append_child(ast, parse_initializer(tokenlist));
            break;
        case CTYPE_FUNC:
            // Do Nothing
            break;
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
    return parse_direct_declarator(tokenlist, ctype);
}

Ast* parse_direct_declarator(TokenList* tokenlist, CType* ctype) {
    Ast* ast = NULL;

    Token* token_ident = assert_and_top_token(tokenlist, TOKEN_IDENT);
    tokenlist_pop(tokenlist);
    Ast* ident = ast_new_ident(AST_IDENT, str_new(token_ident->value_ident));

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_LBRACKET: {
            Ast* array_len_list = parse_array_len_list(tokenlist);
            ident->ctype = ctype;
            size_t i = 0, size = array_len_list->children->size;
            for (i = 0; i < size; i++) {
                Ast* array_len = ast_nth_child(array_len_list, size - i - 1);
                ident->ctype = ctype_new_array(ident->ctype, array_len->value_int);
            }
            ast = ast_new(AST_DECL, 2, ident, array_len_list);
            break;
        }
        case TOKEN_LPAREN: {
            tokenlist_pop(tokenlist);
            Ast* param_list = parse_param_list(tokenlist);
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
    
            Vector* param_ctype_list = vector_new();
            size_t i = 0, size = param_list->children->size;
            vector_reserve(param_ctype_list, size);
            for (i = 0; i < size; i++) {
                Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
                vector_push_back(param_ctype_list, ctype_copy(param_ident->ctype));
            }
            ident->ctype = ctype_new_func(ctype, param_ctype_list);
            ast = ast_new(AST_DECL, 2, ident, param_list);
            break;
        }
        default:
            ident->ctype = ctype;
            ast = ast_new(AST_DECL, 1, ident);
            break;
    }
    // TODO: loop for complex declatations
    return ast;
}

Ast* parse_array_len_list(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_ARRAY_LEN_LIST, 0);
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type != TOKEN_LBRACKET) break;
        tokenlist_pop(tokenlist);

        token = assert_and_top_token(tokenlist, TOKEN_IMM_INT);
        tokenlist_pop(tokenlist);
        ast_append_child(ast, ast_new_int(AST_IMM_INT, token->value_int));
        // TODO: assignment-expr for array length

        assert_and_pop_token(tokenlist, TOKEN_RBRACKET);
    }
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

Ast* parse_initializer(TokenList* tokenlist) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    if (token->type != TOKEN_LBRACE) {
        ast = parse_assignment_expr(tokenlist);
    } else {
        tokenlist_pop(tokenlist);
        ast = parse_initializer_list(tokenlist);
        assert_and_pop_token(tokenlist, TOKEN_RBRACE);
    }
    return ast;
}

Ast* parse_initializer_list(TokenList* tokenlist) {
    Ast* ast = ast_new(AST_INIT_LIST, 0);

    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_RBRACE) return ast;

    ast_append_child(ast, parse_initializer(tokenlist));
    while (1) {
        token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RBRACE) break;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);
        ast_append_child(ast, parse_initializer(tokenlist));
    }
    return ast;
}

// external-declaration-parser
Ast* parse_external_declaration(TokenList* tokenlist) {
    int pos_memo = tokenlist->pos;
    CType* ctype = parse_type_specifier(tokenlist);
    Ast* ast = parse_declarator(tokenlist, ctype);
    Ast* ident = ast_nth_child(ast, 0);

    Token* token = tokenlist_top(tokenlist);
    if (ident->ctype->basic_ctype == CTYPE_FUNC && token->type == TOKEN_LBRACE) {
        return ast_new(AST_FUNC_DEF, 2, ast, parse_compound_stmt(tokenlist));
    }
    tokenlist->pos = pos_memo;
    ast_delete(ast);
    return parse_declaration(tokenlist);
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
