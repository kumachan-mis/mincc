#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

typedef enum {
    TYPE_INT,
    TYPE_PLUS,
    TYPE_MINUS,
    TYPE_EOF
} TOKEN_TYPE;

char TOKEN_PLUS  = '+';
char TOKEN_MINUS = '-';

typedef union {
    int value_int;
} TOKEN_VALUE;

typedef struct {
    TOKEN_TYPE type;
    TOKEN_VALUE value;
} Token;

Token read_token(FILE* file_ptr);
Token read_token_int(FILE* file_ptr);

int main(int argc, char* argv[]) {
    fprintf(stdout, ".global _main\n");
    fprintf(stdout, "_main:\n");

    Token token = read_token(stdin);
    assert(token.type == TYPE_INT);
    fprintf(stdout, "\tmov $%d, %%eax\n", token.value.value_int);
    while (1) {
        token = read_token(stdin);
        if (token.type == TYPE_EOF) break;
    
        if (token.type == TYPE_PLUS) {
            token = read_token(stdin);
            assert(token.type == TYPE_INT);
            fprintf(stdout, "\tadd $%d, %%eax\n", token.value.value_int);
        } else if (token.type == TYPE_MINUS) {
            token = read_token(stdin);
            assert(token.type == TYPE_INT);
            fprintf(stdout, "\tsub $%d, %%eax\n", token.value.value_int);
        } else {
            assert(0);
        }
    }
    printf("\tret\n");
    return 0;
}

Token read_token(FILE* file_ptr) {
    char c = fgetc(file_ptr);
    if (isdigit(c)) {
        ungetc(c, file_ptr);
        return read_token_int(file_ptr);
    } else if (c == TOKEN_PLUS) {
        Token token;
        token.type = TYPE_PLUS;
        return token;
    } else if (c == TOKEN_MINUS) {
        Token token;
        token.type = TYPE_MINUS;
        return token;
    } else {
        Token token;
        token.type = TYPE_EOF;
        return token;
    }
}

Token read_token_int(FILE* file_ptr) { 
    char int_buf[255];
    int buf_pos = 0;
    while (1) {
        char c = fgetc(file_ptr);
        if (!isdigit(c)) {
            ungetc(c, file_ptr);
            break;
        }
        int_buf[buf_pos] = c;
        buf_pos++;
    }
    Token token;
    token.type = TYPE_INT;
    token.value.value_int = atoi(int_buf);
    return token;
}
