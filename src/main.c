#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "mincc_vector.h"
#include "mincc_memory.h"


typedef union {
    int value_int;
} Value;


typedef enum {
    TOKEN_INT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    Value value;
} Token;

Vector* tokenize(FILE* file_ptr);
Token* read_token(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);
Token* token_new(TokenType type);
void tokens_delete(Vector* tokens);


typedef enum {
    AST_INT,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD
} AstType;

typedef struct _Ast {
    AstType type;
    Value value;
    struct _Ast *lhs;
    struct _Ast *rhs;
} Ast;

Ast* parse(Vector* tokens);
Ast* parse_expr(Vector* tokens, size_t* pos);
Ast* parse_primary_expr(Vector* tokens, size_t* pos);
Ast* parse_additive_expr(Vector* tokens, size_t* pos);
Ast* parse_multiplicative_expr(Vector* tokens, size_t* pos);
Ast* ast_new(AstType type);
Ast* ast_delete(Ast* ast);


void print_code(Ast* ast);


int main(int argc, char* argv[]) {
    fprintf(stdout, ".global _main\n");
    fprintf(stdout, "_main:\n");

    Vector* tokens = tokenize(stdin);
    Ast* ast = parse(tokens);
    print_code(ast);

    printf("\tpop %%rax\n");
    printf("\tret\n");

    ast_delete(ast);
    tokens_delete(tokens);

    return 0;
}

Vector* tokenize(FILE* file_ptr) {
    Vector* tokens = vector_new();
    while (1) {
        Token* token = read_token(file_ptr);
        vector_push_back(tokens, token);
        if (token->type == TOKEN_EOF) break;
    }
    return tokens;
}

Token* read_token(FILE* file_ptr) {
    char c = fgetc(file_ptr);
    if (isdigit(c)) {
        ungetc(c, file_ptr);
        return read_token_int(file_ptr);
    }

    Token* token = (Token*)safe_malloc(sizeof(Token));
    switch (c) {
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
        default:
            return token_new(TOKEN_EOF);
    }
    return token;
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

Token* token_new(TokenType type) {
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = type;
    return token;
}

void tokens_delete(Vector* tokens) {
    vector_delete(tokens);
}

Ast* parse(Vector* tokens) {
    size_t pos = 0;
    return parse_expr(tokens, &pos);
}

Ast* parse_expr(Vector* tokens, size_t* pos) {
    return parse_additive_expr(tokens, pos);
}

Ast* parse_primary_expr(Vector* tokens, size_t* pos) {
    Ast* ast;
    Token* token = (Token*)vector_at(tokens, *pos);
    (*pos)++;
    switch (token->type) {
        case TOKEN_INT:
            ast = ast_new(AST_INT);
            ast->value.value_int = token->value.value_int;
            break;
        case TOKEN_LPAREN:
            ast = parse_expr(tokens, pos);
            token = (Token*)vector_at(tokens, *pos);
            (*pos)++;
            assert(token->type == TOKEN_RPAREN);
            break;
        default:
            exit(1);
            break;
    }
    return ast;
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
    Ast* ast = parse_primary_expr(tokens, pos);
    Ast* lhs = NULL;
    Ast* rhs = NULL;
    while (1) {
        Token* token = (Token*)vector_at(tokens, *pos);
        (*pos)++;
        switch (token->type) {
            case TOKEN_ASTERISK:
                lhs = ast;
                rhs = parse_primary_expr(tokens, pos);
                ast = ast_new(AST_MUL);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_SLASH:
                lhs = ast;
                rhs = parse_primary_expr(tokens, pos);
                ast = ast_new(AST_DIV);
                ast->lhs = lhs;
                ast->rhs = rhs;
                break;
            case TOKEN_PERCENT:
                lhs = ast;
                rhs = parse_primary_expr(tokens, pos);
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

Ast* ast_new(AstType type) {
    Ast* ast = (Ast*)safe_malloc(sizeof(Ast));
    ast->type = type;
    ast->lhs = NULL;
    ast->rhs = NULL;
    return ast;
}

Ast* ast_delete(Ast* ast) {
    if (ast->lhs != NULL) ast_delete(ast->lhs);
    if (ast->rhs != NULL) ast_delete(ast->rhs);
    free(ast);
}

void print_code(Ast* ast) {
    if (ast == NULL) return;

    print_code(ast->lhs);
    print_code(ast->rhs);

    if (ast->type == AST_INT) {
        fprintf(stdout, "\tpush $%d\n", ast->value.value_int);
        return;
    }

    fprintf(stdout, "\tpop %%rdi\n");
    fprintf(stdout, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_ADD:
            fprintf(stdout, "\tadd %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_SUB:
            fprintf(stdout, "\tsub %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_MUL:
            fprintf(stdout, "\timul %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_DIV:
            fprintf(stdout, "\tcltd\n");
            fprintf(stdout, "\tidiv %%edi\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_MOD:
            fprintf(stdout, "\tcltd\n");
            fprintf(stdout, "\tidiv %%edi\n");
            fprintf(stdout, "\tpush %%rdx\n");
            break;
        default:
            exit(1);
    }
}