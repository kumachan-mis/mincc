#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "mincc_vector.h"
#include "mincc_map.h"
#include "mincc_memory.h"


typedef union {
    int value_int;
    char* value_ident;
} Value;


typedef enum {
    TOKEN_INT,
    TOKEN_IDENT,
    TOKEN_EQ,
    TOKEN_DBL_AND,
    TOKEN_DBL_BAR,
    TOKEN_EXCL,
    TOKEN_BAR,
    TOKEN_HAT,
    TOKEN_AND,
    TOKEN_TILDER,
    TOKEN_DBL_EQ,
    TOKEN_EXCL_EQ,
    TOKEN_LANGLE,
    TOKEN_RANGLE,
    TOKEN_LANGLE_EQ,
    TOKEN_RANGLE_EQ,
    TOKEN_DBL_LANGLE,
    TOKEN_DBL_RANGLE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_RETURN,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    Value value;
} Token;

Vector* tokenize(FILE* file_ptr);
Token* read_token(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);
Token* read_token_keyword_or_ident(FILE* file_ptr);
void skip_spaces(FILE* file_ptr);
Token* token_new(TokenType type);
void tokens_delete(Vector* tokens);

void assert_lexicon(int condition);


typedef enum {
    AST_INT,
    AST_VAR,
    AST_ASSIGN,
    AST_LAND,
    AST_LOR,
    AST_LNOT,
    AST_OR,
    AST_XOR,
    AST_AND,
    AST_NOT,
    AST_EQ,
    AST_NEQ,
    AST_LT,
    AST_GT,
    AST_LEQ,
    AST_GEQ,
    AST_LSHIFT,
    AST_RSHIFT,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_POSI,
    AST_NEGA,
    AST_RETURN_STMT,
    AST_NULL_STMT,
    AST_EXPR_STMT,
} AstType;

typedef struct _Ast {
    AstType type;
    Value value;
    struct _Ast *lhs;
    struct _Ast *rhs;
} Ast;

Vector* parse(Vector* tokens);
Ast* parse_stmt(Vector* tokens, size_t* pos);
Ast* parse_expr_stmt(Vector* tokens, size_t* pos);
Ast* parse_jump_stmt(Vector* tokens, size_t* pos);
Ast* parse_expr(Vector* tokens, size_t* pos);
Ast* parse_assignment_expr(Vector* tokens, size_t* pos);
Ast* parse_logical_or_expr(Vector* tokens, size_t* pos);
Ast* parse_logical_and_expr(Vector* tokens, size_t* pos);
Ast* parse_or_expr(Vector* tokens, size_t* pos);
Ast* parse_xor_expr(Vector* tokens, size_t* pos);
Ast* parse_and_expr(Vector* tokens, size_t* pos);
Ast* parse_equality_expr(Vector* tokens, size_t* pos);
Ast* parse_relational_expr(Vector* tokens, size_t* pos);
Ast* parse_shift_expr(Vector* tokens, size_t* pos);
Ast* parse_additive_expr(Vector* tokens, size_t* pos);
Ast* parse_multiplicative_expr(Vector* tokens, size_t* pos);
Ast* parse_unary_expr(Vector* tokens, size_t* pos);
Ast* parse_primary_expr(Vector* tokens, size_t* pos);
Ast* ast_new(AstType type);
void ast_delete(Ast* ast);
void asts_delete(Vector* asts);

void assert_syntax(int condition);


typedef struct {
    int num_labels;
    int stack_offset;
    Map* var_map;
    Vector* codes;
} CodeEnvironment;

void print_code(Vector* asts);
void put_code(FILE* file_ptr, Vector* codes);

void gen_stmt_code(Ast* ast, CodeEnvironment* env);
void gen_expr_stmt_code(Ast* ast, CodeEnvironment* env);
void gen_jump_stmt_code(Ast* ast, CodeEnvironment* env);
void gen_expr_code(Ast* ast, CodeEnvironment* env);
void gen_assignment_expr_code(Ast* ast, CodeEnvironment* env);
void gen_logical_expr_code(Ast* ast, CodeEnvironment* env);
void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env);
void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env);
void gen_shift_expr_code(Ast* ast, CodeEnvironment* env);
void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env);
void gen_unary_expr_code(Ast* ast, CodeEnvironment* env);
void gen_primary_expr_code(Ast* ast, CodeEnvironment* env);

int is_expr_stmt(AstType type);
int is_jump_stmt(AstType type);
int is_assignment_expr(AstType type);
int is_logical_expr(AstType type);
int is_bitwise_expr(AstType type);
int is_comparative_expr(AstType type);
int is_shift_expr(AstType type);
int is_arithmetical_expr(AstType type);
int is_unary_expr(AstType type);
int is_primary_expr(AstType type);

void append_code(Vector* codes, char* format, ...);

void assert_code_gen(int condition);


int main(int argc, char* argv[]) {
    Vector* tokens = tokenize(stdin);
    Vector* asts = parse(tokens);
    print_code(asts);
    asts_delete(asts);
    tokens_delete(tokens);
    return 0;
}

Vector* tokenize(FILE* file_ptr) {
    Vector* tokens = vector_new();
    while (1) {
        skip_spaces(file_ptr);
        Token* token = read_token(file_ptr);
        vector_push_back(tokens, token);
        if (token->type == TOKEN_EOF) break;
    }
    return tokens;
}

Token* read_token(FILE* file_ptr) {
    char fst = fgetc(file_ptr);
    if (isdigit(fst)) {
        ungetc(fst, file_ptr);
        return read_token_int(file_ptr);
    } else if (isalpha(fst) || fst == '_') {
        ungetc(fst, file_ptr);
        return read_token_keyword_or_ident(file_ptr);
    }

    char snd = '\0';
    switch (fst) {
        case '|':
            snd = fgetc(file_ptr);
            if (snd == '|') {
                return token_new(TOKEN_DBL_BAR);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_BAR);
            }
        case '^':
            return token_new(TOKEN_HAT);
        case '&':
            snd = fgetc(file_ptr);
            if (snd == '&') {
                return token_new(TOKEN_DBL_AND);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_AND);
            }
        case '~':
            return token_new(TOKEN_TILDER);
        case '=':
            snd = fgetc(file_ptr);
            if (snd == '=') {
                return token_new(TOKEN_DBL_EQ);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_EQ);
            }
        case '!':
            snd = fgetc(file_ptr);
            if (snd == '=') {
                return token_new(TOKEN_EXCL_EQ);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_EXCL);
            }
        case '<':
            snd = fgetc(file_ptr);
            if (snd == '<') {
                return token_new(TOKEN_DBL_LANGLE);
            } else if (snd == '=') {
                return token_new(TOKEN_LANGLE_EQ);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_LANGLE);
            }
        case '>':
            snd = fgetc(file_ptr);
            if (snd == '>') {
                return token_new(TOKEN_DBL_RANGLE);
            } else if (snd == '=') {
                return token_new(TOKEN_RANGLE_EQ);
            } else {
                ungetc(snd, file_ptr);
                return token_new(TOKEN_RANGLE);
            }
        case '+':
            return token_new(TOKEN_PLUS);
        case '-':
            return token_new(TOKEN_MINUS);
        case '*':
            return token_new(TOKEN_ASTERISK);
        case '/':
            return token_new(TOKEN_SLASH);
        case '%':
            return token_new(TOKEN_PERCENT);
        case '(':
            return token_new(TOKEN_LPAREN);
        case ')':
            return token_new(TOKEN_RPAREN);
        case ';':
            return token_new(TOKEN_SEMICOLON);
        case EOF:
            return token_new(TOKEN_EOF);
        default:
            break;
    }
    assert_lexicon(0);
}

Token* read_token_int(FILE* file_ptr) {
    int value = 0;
    while (1) {
        char c = fgetc(file_ptr);
        if (!isdigit(c)) {
            ungetc(c, file_ptr);
            break;
        }
        value = 10 * value + (c - '0');
    }
    Token* token = token_new(TOKEN_INT);
    token->value.value_int = value;
    return token;
}

Token* read_token_keyword_or_ident(FILE* file_ptr) {
    size_t len = 0, capacity = 2;
    char* value = (char*)safe_malloc((capacity)*sizeof(char));

    char c = fgetc(file_ptr);
    assert_lexicon(isalpha(c) || c == '_');
    value[len] = c;
    len++;
    while(1) {
        c = fgetc(file_ptr);
        if (!isalnum(c) && c != '_') {
            ungetc(c, file_ptr);
            break;
        }
        value[len] = c;
        len++;
        if (len + 1 == capacity) {
            capacity *= 2;
            value = (char*)safe_realloc(value, (capacity)*sizeof(char));
        }
    }
    value[len] = '\0';
    value = (char*)safe_realloc(value, (len+1)*sizeof(char));

    Token* token = NULL;
    if (strcmp(value, "return") == 0) {
        token = token_new(TOKEN_RETURN);
    } else {
        token = token_new(TOKEN_IDENT);
        token->value.value_ident = value;
    }
    return token;
}

void skip_spaces(FILE* file_ptr) {
    while (1) {
        char c = fgetc(file_ptr);
        if (!isspace(c)) {
            ungetc(c, file_ptr);
            break;
        }
    }
}

Token* token_new(TokenType type) {
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = type;
    return token;
}

void tokens_delete(Vector* tokens) {
    vector_delete(tokens);
}

void assert_lexicon(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze lexicon\n");
    exit(1);
}

Vector* parse(Vector* tokens) {
    Vector* asts = vector_new();
    size_t pos = 0;
    while (1) {
        Token* token = (Token*)vector_at(tokens, pos);
        if (token->type == TOKEN_EOF) break;
        Ast* ast = parse_stmt(tokens, &pos);
        vector_push_back(asts, ast);
    }
    return asts;
}

Ast* parse_stmt(Vector* tokens, size_t* pos) {
    Token* token = (Token*)vector_at(tokens, *pos);
    if (token->type == TOKEN_RETURN) {
        return parse_jump_stmt(tokens, pos);
    } else {
        return parse_expr_stmt(tokens, pos);
    }
}

Ast* parse_expr_stmt(Vector* tokens, size_t* pos) {
    Ast* ast = NULL;
    Ast* expr = NULL;

    Token* token = (Token*)vector_at(tokens, *pos);
    if (token->type == TOKEN_SEMICOLON) {
        (*pos)++;
        ast = ast_new(AST_NULL_STMT);
    } else {
        expr = parse_expr(tokens, pos);
        token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        assert_syntax(token->type == TOKEN_SEMICOLON);
        ast = ast_new(AST_EXPR_STMT);
        ast->lhs = expr;
    }
    return ast;
}

Ast* parse_jump_stmt(Vector* tokens, size_t* pos) {
    Ast* ast = NULL;
    Ast* expr = NULL;

    Token* token = (Token*)vector_at(tokens, *pos);
    (*pos)++;
    switch (token->type) {
        case TOKEN_RETURN:
            expr = parse_expr(tokens, pos);
            token = (Token*)vector_at(tokens, *pos);
            (*pos)++;
            assert_syntax(token->type == TOKEN_SEMICOLON);
            ast = ast_new(AST_RETURN_STMT);
            ast->lhs = expr;
            break;
        default:
            assert_syntax(0);
            break;
    }
    return ast;
}

Ast* parse_expr(Vector* tokens, size_t* pos) {
    return parse_assignment_expr(tokens, pos);
}

Ast* parse_assignment_expr(Vector* tokens, size_t* pos) {
    size_t pos_memo = *pos;
    Ast* ast = parse_unary_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    Token* token = (Token*)vector_at(tokens, *pos);
    (*pos)++;
    switch (token->type) {
        case TOKEN_EQ:
            lhs = ast;
            rhs = parse_assignment_expr(tokens, pos);
            ast = ast_new(AST_ASSIGN);
            ast->lhs = lhs;
            ast->rhs = rhs;
            break;
        default:
            *pos = pos_memo;
            return parse_logical_or_expr(tokens, pos);
    }
}

Ast* parse_logical_or_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_logical_and_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_DBL_BAR:
                lhs = ast;
                rhs = parse_logical_and_expr(tokens, pos);
                ast = ast_new(AST_LOR);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_logical_and_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_or_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_DBL_AND:
                lhs = ast;
                rhs = parse_or_expr(tokens, pos);
                ast = ast_new(AST_LAND);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_or_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_xor_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_BAR:
                lhs = ast;
                rhs = parse_xor_expr(tokens, pos);
                ast = ast_new(AST_OR);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_xor_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_and_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_HAT:
                lhs = ast;
                rhs = parse_and_expr(tokens, pos);
                ast = ast_new(AST_XOR);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_and_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_equality_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_AND:
                lhs = ast;
                rhs = parse_equality_expr(tokens, pos);
                ast = ast_new(AST_AND);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_equality_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_relational_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_DBL_EQ:
                lhs = ast;
                rhs = parse_relational_expr(tokens, pos);
                ast = ast_new(AST_EQ);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_EXCL_EQ:
                lhs = ast;
                rhs = parse_relational_expr(tokens, pos);
                ast = ast_new(AST_NEQ);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_relational_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_shift_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_LANGLE:
                lhs = ast;
                rhs = parse_shift_expr(tokens, pos);
                ast = ast_new(AST_LT);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_RANGLE:
                lhs = ast;
                rhs = parse_shift_expr(tokens, pos);
                ast = ast_new(AST_GT);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_LANGLE_EQ:
                lhs = ast;
                rhs = parse_shift_expr(tokens, pos);
                ast = ast_new(AST_LEQ);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_RANGLE_EQ:
                lhs = ast;
                rhs = parse_shift_expr(tokens, pos);
                ast = ast_new(AST_GEQ);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_shift_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_additive_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_DBL_LANGLE:
                lhs = ast;
                rhs = parse_additive_expr(tokens, pos);
                ast = ast_new(AST_LSHIFT);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_DBL_RANGLE:
                lhs = ast;
                rhs = parse_additive_expr(tokens, pos);
                ast = ast_new(AST_RSHIFT);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_additive_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_multiplicative_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;

    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_PLUS:
                lhs = ast;
                rhs = parse_multiplicative_expr(tokens, pos);
                ast = ast_new(AST_ADD);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_MINUS:
                lhs = ast;
                rhs = parse_multiplicative_expr(tokens, pos);
                ast = ast_new(AST_SUB);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_multiplicative_expr(Vector* tokens, size_t* pos) {
    Ast* ast = parse_unary_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;
    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_ASTERISK:
                lhs = ast;
                rhs = parse_unary_expr(tokens, pos);
                ast = ast_new(AST_MUL);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_SLASH:
                lhs = ast;
                rhs = parse_unary_expr(tokens, pos);
                ast = ast_new(AST_DIV);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_PERCENT:
                lhs = ast;
                rhs = parse_unary_expr(tokens, pos);
                ast = ast_new(AST_MOD);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            default:
                (*pos)--;
                return ast;
        }
    }
}

Ast* parse_unary_expr(Vector* tokens, size_t* pos) {
    Ast* ast = NULL;
    Ast* lhs = NULL;
    Ast* rhs = NULL;
    Token* token = (Token*)vector_at(tokens, *pos);
    (*pos)++;
    switch (token->type) {
        case TOKEN_PLUS:
            lhs = parse_unary_expr(tokens, pos);
            ast = ast_new(AST_POSI);
            ast->lhs = lhs;
            break;
        case TOKEN_MINUS:
            lhs = parse_unary_expr(tokens, pos);
            ast = ast_new(AST_NEGA);
            ast->lhs = lhs;
            break;
        case TOKEN_TILDER:
            lhs = parse_unary_expr(tokens, pos);
            ast = ast_new(AST_NOT);
            ast->lhs = lhs;
            break;
        case TOKEN_EXCL:
            lhs = parse_unary_expr(tokens, pos);
            ast = ast_new(AST_LNOT);
            ast->lhs = lhs;
            break;
        default:
            (*pos)--;
            ast = parse_primary_expr(tokens, pos);
            break;
    }
    return ast;
}

Ast* parse_primary_expr(Vector* tokens, size_t* pos) {
    Ast* ast = NULL;
    Token* token = (Token*)vector_at(tokens, *pos);
    (*pos)++;
    switch (token->type) {
        case TOKEN_INT:
            ast = ast_new(AST_INT);
            ast->value.value_int = token->value.value_int;
            break;
        case TOKEN_IDENT:
            ast = ast_new(AST_VAR);
            ast->value.value_ident = token->value.value_ident;
            break;
        case TOKEN_LPAREN:
            ast = parse_expr(tokens, pos);
            token = (Token*)vector_at(tokens, *pos);
            (*pos)++;
            assert_syntax(token->type == TOKEN_RPAREN);
            break;
        default:
            assert_syntax(0);
            break;
    }
    return ast;
}

Ast* ast_new(AstType type) {
    Ast* ast = (Ast*)safe_malloc(sizeof(Ast));
    ast->type = type;
    ast->lhs = NULL;
    ast->rhs = NULL;
    return ast;
}

void ast_delete(Ast* ast) {
    if (ast->lhs != NULL) ast_delete(ast->lhs);
    if (ast->rhs != NULL) ast_delete(ast->rhs);
    free(ast);
}

void asts_delete(Vector* asts) {
    size_t i = 0, size = asts->size;
    for (i = 0; i < size; i++) {
        Ast* ast = (Ast*)vector_at(asts, i);
        ast_delete(ast);
    }
    free(asts);
}

void assert_syntax(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to parse input\n");
    exit(1);
}

void print_code(Vector* asts) {
    if (asts->size == 0) return;

    CodeEnvironment env;
    env.num_labels = 0;
    env.stack_offset = 0;
    env.var_map = map_new();
    env.codes = vector_new();

    size_t i = 0, size = asts->size;
    for (i = 0; i < size; i++) {
        gen_stmt_code(vector_at(asts, i), &env);
    }

    Vector* header_codes = vector_new();
    append_code(header_codes, "\t.global _main\n");
    append_code(header_codes, "_main:\n");
    append_code(header_codes, "\tpush %%rbp\n");
    append_code(header_codes, "\tmov %%rsp, %%rbp\n");
    append_code(header_codes, "\tsub $%d, %%rsp\n", env.stack_offset);

    Vector* footer_codes = vector_new();
    append_code(footer_codes, ".L_main_return:\n");
    append_code(footer_codes, "\tmov %%rbp, %%rsp\n");
    append_code(footer_codes, "\tpop %%rbp\n");
    append_code(footer_codes, "\tret\n");

    put_code(stdout, header_codes);
    put_code(stdout, env.codes);
    put_code(stdout, footer_codes);

    vector_delete(footer_codes);
    vector_delete(header_codes);
    vector_delete(env.codes);
    map_delete(env.var_map);
}

void put_code(FILE* file_ptr, Vector* codes) {
    size_t i = 0, size = codes->size;
    for (i = 0; i < size; i++) {
        char* str = (char*)vector_at(codes, i);
        fputs(str, file_ptr);
    }
}

void gen_stmt_code(Ast* ast, CodeEnvironment* env) {
    AstType type = ast->type;

    if (is_expr_stmt(type)) {
        gen_expr_stmt_code(ast, env);
    } else if (is_jump_stmt(type)) {
        gen_jump_stmt_code(ast, env);
    } else  {
        assert_code_gen(0);
    }
}

void gen_expr_stmt_code(Ast* ast, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_EXPR_STMT:
            gen_expr_code(ast->lhs, env);
            append_code(env->codes, "\tadd $8, %%rsp\n");
            break;
        case AST_NULL_STMT:
            append_code(env->codes, "\tnop\n");
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_jump_stmt_code(Ast* ast, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_RETURN_STMT:
            gen_expr_code(ast->lhs, env);
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tjmp .L_main_return\n");
            break;
        default:
            assert_code_gen(0);
            break;
    }
}

void gen_expr_code(Ast* ast, CodeEnvironment* env) {
    AstType type = ast->type;

    if (is_assignment_expr(type)) {
        gen_assignment_expr_code(ast, env);
    } else if (is_logical_expr(type)) {
        gen_logical_expr_code(ast, env);
    } else if (is_bitwise_expr(type)) {
        gen_bitwise_expr_code(ast, env);
    } else if (is_comparative_expr(type)) {
        gen_comparative_expr_code(ast, env);
    } else if (is_shift_expr(type)) {
        gen_shift_expr_code(ast, env);
    } else if (is_arithmetical_expr(type)) {
        gen_arithmetical_expr_code(ast, env);
    } else if (is_unary_expr(type)) {
        gen_unary_expr_code(ast, env);
    } else if (is_primary_expr(type)) {
        gen_primary_expr_code(ast, env);
    } else  {
        assert_code_gen(0);
    }
}

void gen_assignment_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->rhs, env);
    append_code(env->codes, "\tpop %%rax\n");

    assert_code_gen(ast->lhs->type == AST_VAR);
    char* ident = ast->lhs->value.value_ident;
    int* offset_ptr = (int*)map_find(env->var_map, ident);
    if (offset_ptr == NULL) {
        env->stack_offset += 8;
        offset_ptr = int_new(env->stack_offset);
        map_insert(env->var_map, ident, offset_ptr);
    }

    switch (ast->type) {
        case AST_ASSIGN:
            break;
        default:
            assert_code_gen(0);
    }

    append_code(env->codes, "\tmov %%eax, -%d(%%rbp)\n", *offset_ptr);
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_logical_expr_code(Ast* ast, CodeEnvironment* env) {
    int exit_labno = env->num_labels; env->num_labels++;

    gen_expr_code(ast->lhs, env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    switch (ast->type) {
        case AST_LOR:
            append_code(env->codes, "\tjne .L%d\n", exit_labno);
            break;
        case AST_LAND:
            append_code(env->codes, "\tje .L%d\n", exit_labno);
            break;
        default:
            assert_code_gen(0);
    }

    gen_expr_code(ast->rhs, env);
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp $0, %%rax\n");

    append_code(env->codes, ".L%d:\n", exit_labno);
    append_code(env->codes, "\tsetne %%al\n");
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->lhs, env);
    gen_expr_code(ast->rhs, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_OR:
            append_code(env->codes, "\tor %%edi, %%eax\n");
            break;
        case AST_XOR:
            append_code(env->codes, "\txor %%edi, %%eax\n");
            break;
        case AST_AND:
            append_code(env->codes, "\tand %%edi, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->lhs, env);
    gen_expr_code(ast->rhs, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    append_code(env->codes, "\tcmp %%edi, %%eax\n");
    switch (ast->type) {
        case AST_EQ:
            append_code(env->codes, "\tsete %%al\n");
            break;
        case AST_NEQ:
            append_code(env->codes, "\tsetne %%al\n");
            break;
        case AST_LT:
            append_code(env->codes, "\tsetl %%al\n");
            break;
        case AST_GT:
            append_code(env->codes, "\tsetg %%al\n");
            break;
        case AST_LEQ:
            append_code(env->codes, "\tsetle %%al\n");
            break;
        case AST_GEQ:
            append_code(env->codes, "\tsetge %%al\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tmovzb %%al, %%eax\n");
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_shift_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->lhs, env);
    gen_expr_code(ast->rhs, env);

    append_code(env->codes, "\tpop %%rcx\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_LSHIFT:
            append_code(env->codes, "\tsal %%cl, %%eax\n");
            break;
        case AST_RSHIFT:
            append_code(env->codes, "\tsar %%cl, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    append_code(env->codes, "\tpush %%rax\n");
}

void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->lhs, env);
    gen_expr_code(ast->rhs, env);

    append_code(env->codes, "\tpop %%rdi\n");
    append_code(env->codes, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_ADD:
            append_code(env->codes, "\tadd %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_SUB:
            append_code(env->codes, "\tsub %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_MUL:
            append_code(env->codes, "\timul %%edi, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_DIV:
            append_code(env->codes, "\tcltd\n");
            append_code(env->codes, "\tidiv %%edi\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_MOD:
            append_code(env->codes, "\tcltd\n");
            append_code(env->codes, "\tidiv %%edi\n");
            append_code(env->codes, "\tpush %%rdx\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_unary_expr_code(Ast* ast, CodeEnvironment* env) {
    gen_expr_code(ast->lhs, env);

    switch (ast->type) {
        case AST_POSI:
            /* Do Nothing */
            break;
        case AST_NEGA:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tneg %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_NOT:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tnot %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        case AST_LNOT:
            append_code(env->codes, "\tpop %%rax\n");
            append_code(env->codes, "\tcmp $0, %%eax\n");
            append_code(env->codes, "\tsete %%al\n");
            append_code(env->codes, "\tmovzb %%al, %%eax\n");
            append_code(env->codes, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_primary_expr_code(Ast* ast, CodeEnvironment* env) {
    switch (ast->type) {
        case AST_INT:
            append_code(env->codes, "\tpush $%d\n", ast->value.value_int);
            break;
        case AST_VAR: {
            int* offset_ptr = (int*)map_find(env->var_map, ast->value.value_ident);
            assert_code_gen(offset_ptr != NULL);
            append_code(env->codes, "\tmov -%d(%%rbp), %%eax\n", *offset_ptr);
            append_code(env->codes, "\tpush %%rax\n");
        }
        break;
        default:
            assert_code_gen(0);
    }
}

int is_expr_stmt(AstType type) {
    return type == AST_EXPR_STMT || type == AST_NULL_STMT;
}

int is_jump_stmt(AstType type) {
    return type == AST_RETURN_STMT;
}

int is_assignment_expr(AstType type) {
    return type == AST_ASSIGN;
}

int is_logical_expr(AstType type) {
    return type == AST_LAND || type == AST_LOR;
}

int is_bitwise_expr(AstType type) {
    return type == AST_AND || type == AST_XOR || type == AST_OR;
}

int is_comparative_expr(AstType type) {
    return type == AST_EQ  || type == AST_NEQ ||
           type == AST_LT  || type == AST_GT  ||
           type == AST_LEQ || type == AST_GEQ;
}

int is_shift_expr(AstType type) {
    return type == AST_LSHIFT || type == AST_RSHIFT;
}

int is_arithmetical_expr(AstType type) {
    return type == AST_ADD || type == AST_SUB || type == AST_MUL ||
           type == AST_DIV || type == AST_MOD;
}

int is_unary_expr(AstType type) {
    return type == AST_POSI || type == AST_NEGA ||
           type == AST_NOT  || type == AST_LNOT;
}

int is_primary_expr(AstType type) {
    return type == AST_INT || type == AST_VAR;
}

void append_code(Vector* codes, char* format, ...) {
    char buffer[511];
    va_list list;
    va_start(list, format);
    int success = vsnprintf(buffer, 510, format, list);
    va_end(list);
    assert_code_gen(success);
    vector_push_back(codes, str_new(buffer));
}

void assert_code_gen(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to generate code\n");
    exit(1);
}
