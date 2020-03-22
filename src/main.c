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
Ast* ast_delete(Ast* ast);

void assert_syntax(int condition);


typedef struct {
    int num_labels;
} CodeEnvironment;

void print_code(Ast* ast);
void gen_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_logical_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_shift_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_unary_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);
void gen_primary_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr);

int is_logical_expr(AstType type);
int is_bitwise_expr(AstType type);
int is_comparative_expr(AstType type);
int is_shift_expr(AstType type);
int is_arithmetical_expr(AstType type);
int is_unary_expr(AstType type);
int is_primary_expr(AstType type);

void assert_code_gen(int condition);


int main(int argc, char* argv[]) {
    Vector* tokens = tokenize(stdin);
    Ast* ast = parse(tokens);
    print_code(ast);
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
            if (snd == '=') return token_new(TOKEN_DBL_EQ);
            break;
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
    return parse_logical_or_expr(tokens, pos);
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

    CodeEnvironment env;
    env.num_labels = 0;

    fprintf(stdout, ".global _main\n");
    fprintf(stdout, "_main:\n");
    gen_code(ast, &env, stdout);
    printf("\tpop %%rax\n");
    printf("\tret\n");
}

void gen_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    AstType type = ast->type;

    if (is_logical_expr(type)) {
        gen_logical_expr_code(ast, env, file_ptr);
    } else if (is_bitwise_expr(type)) {
        gen_bitwise_expr_code(ast, env, file_ptr);
    } else if (is_comparative_expr(type)) {
        gen_comparative_expr_code(ast, env, file_ptr);
    } else if (is_shift_expr(type)) {
        gen_shift_expr_code(ast, env, file_ptr);
    } else if (is_arithmetical_expr(type)) {
        gen_arithmetical_expr_code(ast, env, file_ptr);
    } else if (is_unary_expr(type)) {
        gen_unary_expr_code(ast, env, file_ptr);
    } else if (is_primary_expr(type)) {
        gen_primary_expr_code(ast, env, file_ptr);
    } else  {
        assert_code_gen(0);
    }
}

void gen_logical_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    int exit_labno = env->num_labels; env->num_labels++;

    gen_code(ast->lhs, env, file_ptr);
    fprintf(file_ptr, "\tpop %%rax\n");
    fprintf(file_ptr, "\tcmp $0, %%rax\n");

    switch (ast->type) {
        case AST_LOR:
            fprintf(file_ptr, "\tjne .L%d\n", exit_labno);
            break;
        case AST_LAND:
            fprintf(file_ptr, "\tje .L%d\n", exit_labno);
            break;
    }

    gen_code(ast->rhs, env, file_ptr);
    fprintf(file_ptr, "\tpop %%rax\n");
    fprintf(file_ptr, "\tcmp $0, %%rax\n");

    fprintf(file_ptr, ".L%d:\n", exit_labno);
    fprintf(file_ptr, "\tsetne %%al\n");
    fprintf(file_ptr, "\tmovzb %%al, %%eax\n");
    fprintf(file_ptr, "\tpush %%rax\n");
}

void gen_bitwise_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    gen_code(ast->lhs, env, file_ptr);
    gen_code(ast->rhs, env, file_ptr);

    fprintf(file_ptr, "\tpop %%rdi\n");
    fprintf(file_ptr, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_OR:
            fprintf(file_ptr, "\tor %%edi, %%eax\n");
            break;
        case AST_XOR:
            fprintf(file_ptr, "\txor %%edi, %%eax\n");
            break;
        case AST_AND:
            fprintf(file_ptr, "\tand %%edi, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    fprintf(file_ptr, "\tpush %%rax\n");
}

void gen_comparative_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    gen_code(ast->lhs, env, file_ptr);
    gen_code(ast->rhs, env, file_ptr);

    fprintf(file_ptr, "\tpop %%rdi\n");
    fprintf(file_ptr, "\tpop %%rax\n");
    fprintf(file_ptr, "\tcmp %%edi, %%eax\n");
    switch (ast->type) {
        case AST_EQ:
            fprintf(file_ptr, "\tsete %%al\n");
            break;
        case AST_NEQ:
            fprintf(file_ptr, "\tsetne %%al\n");
            break;
        case AST_LT:
            fprintf(file_ptr, "\tsetl %%al\n");
            break;
        case AST_GT:
            fprintf(file_ptr, "\tsetg %%al\n");
            break;
        case AST_LEQ:
            fprintf(file_ptr, "\tsetle %%al\n");
            break;
        case AST_GEQ:
            fprintf(file_ptr, "\tsetge %%al\n");
            break;
        default:
            assert_code_gen(0);
    }
    fprintf(file_ptr, "\tmovzb %%al, %%eax\n");
    fprintf(file_ptr, "\tpush %%rax\n");
}

void gen_shift_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    gen_code(ast->lhs, env, file_ptr);
    gen_code(ast->rhs, env, file_ptr);

    fprintf(file_ptr, "\tpop %%rcx\n");
    fprintf(file_ptr, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_LSHIFT:
            fprintf(file_ptr, "\tsal %%cl, %%eax\n");
            break;
        case AST_RSHIFT:
            fprintf(file_ptr, "\tsar %%cl, %%eax\n");
            break;
        default:
            assert_code_gen(0);
    }
    fprintf(file_ptr, "\tpush %%rax\n");
}

void gen_arithmetical_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    gen_code(ast->lhs, env, file_ptr);
    gen_code(ast->rhs, env, file_ptr);

    fprintf(file_ptr, "\tpop %%rdi\n");
    fprintf(file_ptr, "\tpop %%rax\n");
    switch (ast->type) {
        case AST_ADD:
            fprintf(file_ptr, "\tadd %%edi, %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_SUB:
            fprintf(file_ptr, "\tsub %%edi, %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_MUL:
            fprintf(file_ptr, "\timul %%edi, %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_DIV:
            fprintf(file_ptr, "\tcltd\n");
            fprintf(file_ptr, "\tidiv %%edi\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_MOD:
            fprintf(file_ptr, "\tcltd\n");
            fprintf(file_ptr, "\tidiv %%edi\n");
            fprintf(file_ptr, "\tpush %%rdx\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_unary_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    gen_code(ast->lhs, env, file_ptr);

    switch (ast->type) {
        case AST_POSI:
            /* Do Nothing */
            break;
        case AST_NEGA:
            fprintf(file_ptr, "\tpop %%rax\n");
            fprintf(file_ptr, "\tneg %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_NOT:
            fprintf(file_ptr, "\tpop %%rax\n");
            fprintf(file_ptr, "\tnot %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        case AST_LNOT:
            fprintf(file_ptr, "\tpop %%rax\n");
            fprintf(file_ptr, "\tcmp $0, %%eax\n");
            fprintf(file_ptr, "\tsete %%al\n");
            fprintf(file_ptr, "\tmovzb %%al, %%eax\n");
            fprintf(file_ptr, "\tpush %%rax\n");
            break;
        default:
            assert_code_gen(0);
    }
}

void gen_primary_expr_code(Ast* ast, CodeEnvironment* env, FILE* file_ptr) {
    switch (ast->type) {
        case AST_INT:
            fprintf(file_ptr, "\tpush $%d\n", ast->value.value_int);
            break;
        default:
            assert_code_gen(0);
    }
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
    return type == AST_INT;
}

void assert_code_gen(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to generate code\n");
    exit(1);
}
