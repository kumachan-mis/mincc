#ifndef _TOKEN_H_
#define _TOKEN_H_


#include "../common/vector.h"


typedef enum {
    // keyword
    TOKEN_BREAK,
    TOKEN_CHAR,
    TOKEN_CONTINUE,
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
    TOKEN_IMM_STR,

    // punctuator
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_DBL_PLUS,
    TOKEN_DBL_MINUS,
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
    TOKEN_ASTERISK_EQ,
    TOKEN_SLASH_EQ,
    TOKEN_PERCENT_EQ,
    TOKEN_PLUS_EQ,
    TOKEN_MINUS_EQ,
    TOKEN_DBL_LANGLE_EQ,
    TOKEN_DBL_RANGLE_EQ,
    TOKEN_AND_EQ,
    TOKEN_HAT_EQ,
    TOKEN_BAR_EQ,
    TOKEN_COMMA,

    // eof
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    union {
        int value_int;
        char* value_str;
        char* value_ident;
    };
} Token;

typedef struct {
    Vector* inner_vector;
    int pos;
} TokenList;

 typedef struct {
    char* value_token;
    TokenType type;
 } ReservedTokenEntry;

typedef Vector ReservedTokenList;


// tokenlist
TokenList* tokenlist_new();
void tokenlist_append(TokenList* tokenlist, Token* token);
Token* tokenlist_top(TokenList* tokenlist);
void tokenlist_pop(TokenList* tokenlist);
void tokenlist_delete(TokenList* tokenlist);

// token
Token* token_new(TokenType type);
Token* token_new_int(TokenType type, int value_int);
Token* token_new_str(TokenType type, char* value_str);
Token* token_new_ident(TokenType type, char* value_ident);
void token_delete(Token* token);

// reserved-token-list
ReservedTokenList* reserved_token_list_new_punctuators();
ReservedTokenList* reserved_token_list_new_keywords();
ReservedTokenEntry* reserved_token_list_at(ReservedTokenList* reserved_token_list, size_t index);
void reserved_token_list_delete(ReservedTokenList* reserved_token_list);


#endif  // _TOKEN_H_
