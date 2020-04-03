#include "parser.h"

#include <stdlib.h>
#include <stdarg.h>
#include "lex.h"
#include "symboltable.h"
#include "memory.h"

// expression-parser
Ast* parse_primary_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_postfix_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_arg_expr_list(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_unary_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_multiplicative_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_additive_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_shift_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_relational_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_equality_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_and_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_xor_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_or_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_logical_and_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_logical_or_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_assignment_expr(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_expr(TokenList* tokenlist, SymbolTable* symbol_table);

// statement-parser
Ast* parse_compound_stmt(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_expr_stmt(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_selection_stmt(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_iteration_stmt(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_jump_stmt(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_stmt(TokenList* tokenlist, SymbolTable* symbol_table);

// declaration-parser
Ast* parse_declaration(TokenList* tokenlist, SymbolTable* symbol_table);
CType* parse_type_specifier(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_init_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* basic_ctype);
Ast* parse_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* basic_ctype);
Ast* parse_direct_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* ctype);
Ast* parse_param_list(TokenList* tokenlist, SymbolTable* symbol_table);
Ast* parse_param_declaration(TokenList* tokenlist, SymbolTable* symbol_table);

// external-definition-parser
Ast* parse_function_definition(TokenList* tokenlist, SymbolTable* symbol_table);

// ast
AstList* astlist_new();
Ast* ast_new(AstType type, size_t num_children, ...);
Ast* ast_new_int(AstType type, int value_int);
Ast* ast_new_ident(AstType type, char* value_ident, CType* ctype);
void ast_append_child(Ast* ast, Ast* child);
void ast_delete(Ast* ast);

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
        Ast* ast = parse_function_definition(tokenlist, NULL);
        vector_push_back(inner_vector, ast);
    }
    return astlist;
}

// expression-parser
Ast* parse_primary_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;
    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);

    switch (token->type) {
        case TOKEN_IMM_INT:
            ast = ast_new_int(AST_IMM_INT, token->value_int);
            break;
        case TOKEN_IDENT:
            ast = ast_new_ident(
                AST_IDENT,
                str_new(token->value_ident),
                ctype_copy(symbol_table_get_ctype(symbol_table, token->value_ident))
            );
            break;
        case TOKEN_LPAREN:
            ast = parse_expr(tokenlist, symbol_table);
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_postfix_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_primary_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_LPAREN:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_FUNC_CALL, 2, ast, parse_arg_expr_list(tokenlist, symbol_table));
                assert_and_pop_token(tokenlist, TOKEN_RPAREN);
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_arg_expr_list(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = ast_new(AST_ARG_LIST, 0);
    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_RPAREN) return ast;
    
    ast_append_child(ast, parse_assignment_expr(tokenlist, symbol_table));
    while (1) {
        token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RPAREN) return ast;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);
        ast_append_child(ast, parse_assignment_expr(tokenlist, symbol_table));
    }
}

Ast* parse_unary_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_AND:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_ADDR, 1, parse_unary_expr(tokenlist, symbol_table));
            break;
        case TOKEN_ASTERISK:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_DEREF, 1, parse_unary_expr(tokenlist, symbol_table));
            break;
        case TOKEN_PLUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_POSI, 1, parse_unary_expr(tokenlist, symbol_table));
            break;
        case TOKEN_MINUS:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_NEGA, 1, parse_unary_expr(tokenlist, symbol_table));
            break;
        case TOKEN_TILDER:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_NOT,  1, parse_unary_expr(tokenlist, symbol_table));
            break;
        case TOKEN_EXCL:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_LNOT, 1, parse_unary_expr(tokenlist, symbol_table));
            break;
        default:
            ast = parse_postfix_expr(tokenlist, symbol_table);
            break;
    }
    return ast;
}

Ast* parse_multiplicative_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_unary_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_ASTERISK:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_MUL, 2, ast, parse_unary_expr(tokenlist, symbol_table));
                break;
            case TOKEN_SLASH:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_DIV, 2, ast, parse_unary_expr(tokenlist, symbol_table));
                break;
            case TOKEN_PERCENT:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_MOD, 2, ast, parse_unary_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_additive_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_multiplicative_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_PLUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_ADD, 2, ast, parse_multiplicative_expr(tokenlist, symbol_table));
                break;
            case TOKEN_MINUS:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_SUB, 2, ast, parse_multiplicative_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_shift_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_additive_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_LANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LSHIFT, 2, ast, parse_additive_expr(tokenlist, symbol_table));
                break;
            case TOKEN_DBL_RANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_RSHIFT, 2, ast, parse_additive_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_relational_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_shift_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_LANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LT, 2, ast, parse_shift_expr(tokenlist, symbol_table));
                break;
            case TOKEN_RANGLE:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_GT, 2, ast, parse_shift_expr(tokenlist, symbol_table));
                break;
            case TOKEN_LANGLE_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LEQ, 2, ast, parse_shift_expr(tokenlist, symbol_table));
                break;
            case TOKEN_RANGLE_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_GEQ, 2, ast, parse_shift_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_equality_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_relational_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_EQ, 2, ast, parse_relational_expr(tokenlist, symbol_table));
                break;
            case TOKEN_EXCL_EQ:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_NEQ, 2, ast, parse_relational_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_and_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_equality_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_AND:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_AND, 2, ast, parse_equality_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_xor_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_and_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_HAT:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_XOR, 2, ast, parse_and_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_or_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_xor_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_BAR:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_OR, 2, ast, parse_xor_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_logical_and_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_or_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_AND:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LAND, 2, ast, parse_or_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_logical_or_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = parse_logical_and_expr(tokenlist, symbol_table);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        switch (token->type) {
            case TOKEN_DBL_BAR:
                tokenlist_pop(tokenlist);
                ast = ast_new(AST_LOR, 2, ast, parse_logical_and_expr(tokenlist, symbol_table));
                break;
            default:
                return ast;
        }
    }
}

Ast* parse_assignment_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    size_t pos_memo = tokenlist->pos;
    Ast* ast = parse_unary_expr(tokenlist, symbol_table);

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_EQ:
            tokenlist_pop(tokenlist);
            ast = ast_new(AST_ASSIGN, 2, ast, parse_assignment_expr(tokenlist, symbol_table));
            break;
        default:
            tokenlist->pos = pos_memo;
            ast = parse_logical_or_expr(tokenlist, symbol_table);
            break;
    }
    return ast;
}

Ast* parse_expr(TokenList* tokenlist, SymbolTable* symbol_table) {
    return parse_assignment_expr(tokenlist, symbol_table);
}

// statement-parser
Ast* parse_compound_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = ast_new(AST_COMP_STMT, 0);

    assert_and_pop_token(tokenlist, TOKEN_LBRACE);
    SymbolTable* block_scope_table = symbol_table_new();
    symbol_table_enter_into_scope(block_scope_table, symbol_table);
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RBRACE) {
            symbol_table_exit_from_scope(block_scope_table, symbol_table);
            tokenlist_pop(tokenlist);
            ast->symbol_table = block_scope_table;
            break;
        }

        if (token->type == TOKEN_INT) {
            ast_append_child(ast, parse_declaration(tokenlist, block_scope_table));
        } else {
            ast_append_child(ast, parse_stmt(tokenlist, block_scope_table));
        }
    }
    return ast;
}

Ast* parse_expr_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = ast_new(AST_EXPR_STMT, 0);

    Token* token = tokenlist_top(tokenlist);
    ast_append_child(
        ast,
        token->type == TOKEN_SEMICOLON
        ? ast_new(AST_NULL, 0)
        : parse_expr(tokenlist, symbol_table)
    );
    assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
    return ast;
}

Ast* parse_selection_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_IF:
            ast = ast_new(AST_IF_STMT, 0);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist, symbol_table));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            ast_append_child(ast, parse_stmt(tokenlist, symbol_table));

            token = tokenlist_top(tokenlist);
            if (token->type == TOKEN_ELSE) {
                tokenlist_pop(tokenlist);
                ast_append_child(ast, parse_stmt(tokenlist, symbol_table));
            }
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_iteration_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_WHILE:
            ast = ast_new(AST_WHILE_STMT, 0);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist, symbol_table));
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            ast_append_child(ast, parse_stmt(tokenlist, symbol_table));
            break;
        case TOKEN_DO:
            ast = ast_new(AST_DOWHILE_STMT, 0);
            ast_append_child(ast, parse_stmt(tokenlist, symbol_table));
            assert_and_pop_token(tokenlist, TOKEN_WHILE);
            assert_and_pop_token(tokenlist, TOKEN_LPAREN);
            ast_append_child(ast, parse_expr(tokenlist, symbol_table));
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
                : parse_expr(tokenlist, symbol_table)
            );
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            token = tokenlist_top(tokenlist);
            ast_append_child(
                ast,
                token->type == TOKEN_SEMICOLON
                ? ast_new(AST_NULL, 0)
                : parse_expr(tokenlist, symbol_table)
            );
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            token = tokenlist_top(tokenlist);
            ast_append_child(
                ast,
                token->type == TOKEN_RPAREN
                ? ast_new(AST_NULL, 0)
                : parse_expr(tokenlist, symbol_table)
            );
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            ast_append_child(ast, parse_stmt(tokenlist, symbol_table));
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_jump_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;

    Token* token = tokenlist_top(tokenlist);
    tokenlist_pop(tokenlist);
    switch (token->type) {
        case TOKEN_RETURN:
            ast = ast_new(AST_RETURN_STMT, 0);
            ast_append_child(ast, parse_expr(tokenlist, symbol_table));
            assert_and_pop_token(tokenlist, TOKEN_SEMICOLON);
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_stmt(TokenList* tokenlist, SymbolTable* symbol_table) {
    Token* token = tokenlist_top(tokenlist);
    TokenType type = token->type;
    if (type == TOKEN_LBRACE) {
        return parse_compound_stmt(tokenlist, symbol_table);
    } else if (type == TOKEN_IF) {
        return parse_selection_stmt(tokenlist, symbol_table);
    } else if (
        type == TOKEN_WHILE ||
        type == TOKEN_DO ||
        type == TOKEN_FOR
    ) {
        return parse_iteration_stmt(tokenlist, symbol_table);
    } else if (type == TOKEN_RETURN) {
        return parse_jump_stmt(tokenlist, symbol_table);
    } else {
        return parse_expr_stmt(tokenlist, symbol_table);
    }
}

// declaration-parser
Ast* parse_declaration(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = ast_new(AST_DECL_LIST, 0);
    CType* basic_ctype = parse_type_specifier(tokenlist, symbol_table);

    Ast* child = parse_init_declarator(tokenlist, symbol_table, ctype_copy(basic_ctype));
    Ast* ident = ast_nth_child(child, 0);
    symbol_table_insert(symbol_table, str_new(ident->value_ident), ctype_copy(ident->ctype));
    ast_append_child(ast, child);

    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_SEMICOLON) break;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);

        child = parse_init_declarator(tokenlist, symbol_table, ctype_copy(basic_ctype));
        ident = ast_nth_child(child, 0);
        symbol_table_insert(symbol_table, str_new(ident->value_ident), ctype_copy(ident->ctype));
        ast_append_child(ast, child);
    }

    ctype_delete(basic_ctype);
    return ast;
}

CType* parse_type_specifier(TokenList* tokenlist, SymbolTable* symbol_table) {
    CType* ctype = NULL;

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_INT:
            tokenlist_pop(tokenlist);
            ctype = ctype_new(CTYPE_INT);
            break;
        default:
            assert_syntax(0);
    }
    return ctype;
}

Ast* parse_init_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* basic_ctype) {
    Ast* ast = parse_declarator(tokenlist, symbol_table, basic_ctype);

    Token* token = tokenlist_top(tokenlist);
    switch (ast->type) {
        case AST_IDENT_DECL:
            if (token->type == TOKEN_EQ) {
                tokenlist_pop(tokenlist);
                ast_append_child(ast, parse_assignment_expr(tokenlist, symbol_table));
            } else {
                ast_append_child(ast, ast_new(AST_NULL, 0));
            }
            break;
        default:
            assert_syntax(0);
    }
    return ast;
}

Ast* parse_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* basic_ctype) {
    CType* ctype = basic_ctype;
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type != TOKEN_ASTERISK) break;
        tokenlist_pop(tokenlist);
        ctype = ctype_new_ptr(ctype);
    }
    Ast* ast = parse_direct_declarator(tokenlist, symbol_table, ctype);
    return ast;
}

Ast* parse_direct_declarator(TokenList* tokenlist, SymbolTable* symbol_table, CType* ctype) {
    Ast* ast = NULL;

    Token* token_ident = assert_and_top_token(tokenlist, TOKEN_IDENT);
    tokenlist_pop(tokenlist);

    Token* token = tokenlist_top(tokenlist);
    switch (token->type) {
        case TOKEN_LPAREN:
            tokenlist_pop(tokenlist);
            ast = ast_new(
                AST_FUNC_DECL, 2,
                ast_new_ident(AST_IDENT, str_new(token_ident->value_ident), ctype),
                parse_param_list(tokenlist, symbol_table)
            );
            assert_and_pop_token(tokenlist, TOKEN_RPAREN);
            break;
        default:
            ast = ast_new(
                AST_IDENT_DECL, 1,
                ast_new_ident(AST_IDENT, str_new(token_ident->value_ident), ctype)
            );
            break;
    }
    return ast;
}

Ast* parse_param_list(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = ast_new(AST_PARAM_LIST, 0);
    Token* token = tokenlist_top(tokenlist);
    if (token->type == TOKEN_RPAREN) return ast;

    ast_append_child(ast, parse_param_declaration(tokenlist, symbol_table));
    while (1) {
        token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RPAREN) break;
        assert_and_pop_token(tokenlist, TOKEN_COMMA);
        ast_append_child(ast, parse_param_declaration(tokenlist, symbol_table));
    }
    return ast;
}

Ast* parse_param_declaration(TokenList* tokenlist, SymbolTable* symbol_table) {
    CType* basic_ctype = parse_type_specifier(tokenlist, symbol_table);
    Ast* ast = parse_declarator(tokenlist, symbol_table, basic_ctype);
    assert_syntax(ast->type == AST_IDENT_DECL);
    return ast;
}

// external-definition-parser
Ast* parse_function_definition(TokenList* tokenlist, SymbolTable* symbol_table) {
    Ast* ast = NULL;
    CType* ctype = parse_type_specifier(tokenlist, symbol_table);
    Ast* decl = parse_declarator(tokenlist, symbol_table, ctype);
    assert_syntax(decl->type == AST_FUNC_DECL);
   
    SymbolTable* func_scope_table = symbol_table_new();
    symbol_table_enter_into_scope(func_scope_table, symbol_table);

    Ast* param_list = ast_nth_child(decl, 1);
    size_t i = 0, size = param_list->children->size;
    for (i = 0; i < size; i++) {
        Ast* param_ident = ast_nth_child(ast_nth_child(param_list, i), 0);
        symbol_table_insert(
            func_scope_table,
            str_new(param_ident->value_ident),
            ctype_copy(param_ident->ctype)
        );
    }

    Ast* block = ast_new(AST_COMP_STMT, 0);
    assert_and_pop_token(tokenlist, TOKEN_LBRACE);
    while (1) {
        Token* token = tokenlist_top(tokenlist);
        if (token->type == TOKEN_RBRACE) {
            tokenlist_pop(tokenlist);
            symbol_table_exit_from_scope(func_scope_table, symbol_table);
            block->symbol_table = func_scope_table;
            break;
        }
        if (token->type == TOKEN_INT) {
            ast_append_child(block, parse_declaration(tokenlist, func_scope_table));
        } else {
            ast_append_child(block, parse_stmt(tokenlist, func_scope_table));
        }
    }
    ast = ast_new(AST_FUNC_DEF, 2, decl, block);
            
    return ast;
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
    ast->ctype = ctype_new(CTYPE_INT);
    return ast;
}

Ast* ast_new_ident(AstType type, char* value_ident, CType* ctype) {
    Ast* ast = ast_new(type, 0);
    ast->value_ident = value_ident;
    ast->ctype = ctype;
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
