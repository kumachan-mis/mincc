#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "mincc_vector.h"
#include "mincc_memory.h"


typedef union {
    int value_int;
} Value;


typedef enum {
    TOKEN_INT,
    TOKEN_BAR,
    TOKEN_HAT,
    TOKEN_AND,
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
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    Value value;
} Token;

Vector* tokenize(FILE* file_ptr);
Token* read_token(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);
void skip_spaces(FILE* file_ptr);
Token* token_new(TokenType type);
void tokens_delete(Vector* tokens);
void assert_lexicon(int condition);


typedef enum {
    AST_INT,
    AST_OR,
    AST_XOR,
    AST_AND,
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
    AST_NEGA
} AstType;

typedef struct _Ast {
    AstType type;
    Value value;
    struct _Ast *lhs;
    struct _Ast *rhs;
} Ast;

Ast* parse(Vector* tokens);
Ast* parse_expr(Vector* tokens, size_t* pos);
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
Ast* ast_delete(Ast* ast);
void assert_syntax(int condition);


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
    }

    char snd = '\0';
    switch (fst) {
        case '|':
            return token_new(TOKEN_BAR);
        case '^':
            return token_new(TOKEN_HAT);
        case '&':
            return token_new(TOKEN_AND);
        case '=':
            snd = fgetc(file_ptr);
            if (snd == '=') return token_new(TOKEN_DBL_EQ);
            break;
        case '!':
            snd = fgetc(file_ptr);
            if (snd == '=') return token_new(TOKEN_EXCL_EQ);
            break;
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

Ast* parse(Vector* tokens) {
    size_t pos = 0;
    return parse_expr(tokens, &pos);
}

Ast* parse_expr(Vector* tokens, size_t* pos) {
    return parse_or_expr(tokens, pos);
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

Ast* ast_delete(Ast* ast) {
    if (ast->lhs != NULL) ast_delete(ast->lhs);
    if (ast->rhs != NULL) ast_delete(ast->rhs);
    free(ast);
}

void assert_syntax(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to parse input\n");
    exit(1);
}

void print_code(Ast* ast) {
    if (ast == NULL) return;

    print_code(ast->lhs);
    print_code(ast->rhs);

    switch (ast->type) {
        case AST_OR:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tor %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_XOR:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\txor %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_AND:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tand %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_EQ:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsete %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_NEQ:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsetne %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_LT:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsetl %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_GT:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsetg %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_LEQ:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsetle %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_GEQ:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcmp %%edi, %%eax\n");
            fprintf(stdout, "\tsetge %%al\n");
            fprintf(stdout, "\tmovzb %%al, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_LSHIFT:
            fprintf(stdout, "\tpop %%rcx\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tsal %%cl, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_RSHIFT:
            fprintf(stdout, "\tpop %%rcx\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tsar %%cl, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_ADD:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tadd %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_SUB:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tsub %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_MUL:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\timul %%edi, %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_DIV:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcltd\n");
            fprintf(stdout, "\tidiv %%edi\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_MOD:
            fprintf(stdout, "\tpop %%rdi\n");
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tcltd\n");
            fprintf(stdout, "\tidiv %%edi\n");
            fprintf(stdout, "\tpush %%rdx\n");
            break;
        case AST_POSI:
            break;
        case AST_NEGA:
            fprintf(stdout, "\tpop %%rax\n");
            fprintf(stdout, "\tneg %%eax\n");
            fprintf(stdout, "\tpush %%rax\n");
            break;
        case AST_INT:
            fprintf(stdout, "\tpush $%d\n", ast->value.value_int);
            break;
        default:
            exit(1);
    }
}