#include "token.h"

#include <stdlib.h>
#include "../common/memory.h"


// tokenlist
TokenList* tokenlist_new() {
    TokenList* tokenlist = (TokenList*)safe_malloc(sizeof(TokenList));
    tokenlist->inner_vector = vector_new();
    tokenlist->pos = 0;
    return tokenlist;
}

Token* tokenlist_top(TokenList* tokenlist) {
    return (Token*)vector_at(tokenlist->inner_vector, tokenlist->pos);
}

void tokenlist_pop(TokenList* tokenlist) {
    tokenlist->pos++;
}

void tokenlist_delete(TokenList* tokenlist) {
    Vector* inner_vector = tokenlist->inner_vector;
    size_t i = 0, size = inner_vector->size;
    for (i = 0; i < size; i++) {
        token_delete(vector_at(inner_vector, i));
        inner_vector->data[i] = NULL;
    }
    vector_delete(inner_vector);
    free(tokenlist);
}

// token
Token* token_new(TokenType type) {
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = type;
    return token;
}

Token* token_new_int(TokenType type, int value_int) {
    Token* token = token_new(type);
    token->value_int = value_int;
    return token;
}

Token* token_new_ident(TokenType type, char* value_ident) {
    Token* token = token_new(type);
    token->value_ident = value_ident;
    return token;
}

void token_delete(Token* token) {
    if (token->type == TOKEN_IDENT) {
        free(token->value_ident);
    }
    free(token);
}
