#ifndef _LEX_H_
#define _LEX_H_


#include <stdio.h>
#include "vector.h"


typedef enum {
    // keyword
    TOKEN_RETURN,

    // identifier
    TOKEN_IDENT,

    // constant
    TOKEN_INT_CONST,

    // punctuator
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_AND,
    TOKEN_ASTERISK,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TILDER,
    TOKEN_EXCL,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_DBL_LANGLE,
    TOKEN_DBL_RANGLE,
    TOKEN_LANGLE,
    TOKEN_RANGLE,
    TOKEN_LANGLE_EQ,
    TOKEN_RANGLE_EQ,
    TOKEN_DBL_EQ,
    TOKEN_EXCL_EQ,
    TOKEN_HAT,
    TOKEN_BAR,
    TOKEN_DBL_AND,
    TOKEN_DBL_BAR,
    TOKEN_SEMICOLON,
    TOKEN_EQ,

    // eof
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    union {
        int value_int;
        char* value_ident;
    };
} Token;

typedef struct {
    Vector* inner_vector;
    int pos;
} TokenList;

TokenList* tokenize(FILE* file_ptr);

Token* tokenlist_top(TokenList* tokenlist);
void tokenlist_pop(TokenList* tokenlist);
void tokenlist_delete(TokenList* tokenlist);


#endif // _LEX_H_
