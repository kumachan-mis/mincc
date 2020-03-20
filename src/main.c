#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "mincc_vector.h"
#include "mincc_memory.h"


typedef enum {
    TYPE_INT,
    TYPE_PLUS,
    TYPE_MINUS,
    TYPE_EOF
} TOKEN_TYPE;

typedef union {
    int value_int;
} TOKEN_VALUE;

typedef struct {
    TOKEN_TYPE type;
    TOKEN_VALUE value;
} Token;

Vector* read_tokens(FILE* file_ptr);
Token* read_token(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);

int main(int argc, char* argv[]) {
    fprintf(stdout, ".global _main\n");
    fprintf(stdout, "_main:\n");

    Vector* tokens = read_tokens(stdin);
    size_t i = 0;

    Token* token = (Token*)vector_at(tokens, i);
    i++;
    assert(token->type == TYPE_INT);
    fprintf(stdout, "\tmov $%d, %%eax\n", token->value.value_int);
    while (1) {
        token = (Token*)vector_at(tokens, i);
        i++;
        if (token->type == TYPE_EOF) break;
        switch (token->type) {
            case TYPE_PLUS:
                token = (Token*)vector_at(tokens, i);
                i++;
                assert(token->type == TYPE_INT);
                fprintf(stdout, "\tadd $%d, %%eax\n", token->value.value_int);
                break;
            case TYPE_MINUS:
                token = (Token*)vector_at(tokens, i);
                i++;
                assert(token->type == TYPE_INT);
                fprintf(stdout, "\tsub $%d, %%eax\n", token->value.value_int);
                break;
            default:
                fprintf(stderr, "Error: unexpected token\n");
                exit(1);
        }
    
    }
    printf("\tret\n");
    vector_delete(tokens);
    return 0;
}

Vector* read_tokens(FILE* file_ptr) {
    Vector* tokens = vector_new();
    while (1) {
        Token* token = read_token(file_ptr);
        vector_push_back(tokens, token);
        if (token->type == TYPE_EOF) break;
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
            token->type = TYPE_PLUS;
            break;
        case '-':
            token->type = TYPE_MINUS;
            break;
        default:
            token->type = TYPE_EOF;
            break;
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
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = TYPE_INT;
    token->value.value_int = value;
    return token;
}
