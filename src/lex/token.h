#ifndef _TOKEN_H_
#define _TOKEN_H_


#include "../common/vector.h"


typedef enum {
    // keyword
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_IF,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_WHILE,

    // identifier
    TOKEN_IDENT,

    // constant
    TOKEN_IMM_INT,

    // punctuator
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
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
    TOKEN_COMMA,

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


// tokenlist
TokenList* tokenlist_new();
Token* tokenlist_top(TokenList* tokenlist);
void tokenlist_pop(TokenList* tokenlist);
void tokenlist_delete(TokenList* tokenlist);

// token
Token* token_new(TokenType type);
Token* token_new_int(TokenType type, int value_int);
Token* token_new_ident(TokenType type, char* value_ident);
void token_delete(Token* token);


#endif  // _TOKEN_H_
