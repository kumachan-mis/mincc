#include "token.h"

#include <stdlib.h>
#include "../common/memory.h"


// reserved-token-list
void reserved_token_list_append(ReservedTokenList* reserved_token_list, char* value_token, TokenType type);

// tokenlist
TokenList* tokenlist_new() {
    TokenList* tokenlist = (TokenList*)safe_malloc(sizeof(TokenList));
    tokenlist->inner_vector = vector_new();
    tokenlist->pos = 0;
    return tokenlist;
}

void tokenlist_append(TokenList* tokenlist, Token* token) {
    vector_push_back(tokenlist->inner_vector, token);
}

Token* tokenlist_top(TokenList* tokenlist) {
    return (Token*)vector_at(tokenlist->inner_vector, tokenlist->pos);
}

void tokenlist_pop(TokenList* tokenlist) {
    tokenlist->pos++;
}

void tokenlist_delete(TokenList* tokenlist) {
    if (tokenlist == NULL) return;

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

Token* token_new_str(TokenType type, char* value_str) {
    Token* token = token_new(type);
    token->value_str = value_str;
    return token;
}

Token* token_new_ident(TokenType type, char* value_ident) {
    Token* token = token_new(type);
    token->value_ident = value_ident;
    return token;
}

void token_delete(Token* token) {
    if (token == NULL) return;

    switch (token->type) {
        case TOKEN_IDENT:
            free(token->value_ident);
            break;
        case TOKEN_IMM_STR:
            free(token->value_str);
            break;
        default:
            break;
    }
    free(token);
}

// reserved-token-list
ReservedTokenList* reserved_token_list_new_punctuators() {
    size_t size = 39;
    ReservedTokenList* punctuator_list = vector_new();
    vector_reserve(punctuator_list, size);

    reserved_token_list_append(punctuator_list, "(",   TOKEN_LPAREN);
    reserved_token_list_append(punctuator_list, ")",   TOKEN_RPAREN);
    reserved_token_list_append(punctuator_list, "{",   TOKEN_LBRACE);
    reserved_token_list_append(punctuator_list, "}",   TOKEN_RBRACE);
    reserved_token_list_append(punctuator_list, "[",   TOKEN_LBRACKET);
    reserved_token_list_append(punctuator_list, "]",   TOKEN_RBRACKET);
    reserved_token_list_append(punctuator_list, ",",   TOKEN_COMMA);
    reserved_token_list_append(punctuator_list, ";",   TOKEN_SEMICOLON);
    reserved_token_list_append(punctuator_list, "||",  TOKEN_DBL_BAR);
    reserved_token_list_append(punctuator_list, "|=",  TOKEN_BAR_EQ);
    reserved_token_list_append(punctuator_list, "|",   TOKEN_BAR);
    reserved_token_list_append(punctuator_list, "^=",  TOKEN_HAT_EQ);
    reserved_token_list_append(punctuator_list, "^",   TOKEN_HAT);
    reserved_token_list_append(punctuator_list, "&&",  TOKEN_DBL_AND);
    reserved_token_list_append(punctuator_list, "&=",  TOKEN_AND_EQ);
    reserved_token_list_append(punctuator_list, "&",   TOKEN_AND);
    reserved_token_list_append(punctuator_list, "~",   TOKEN_TILDER);
    reserved_token_list_append(punctuator_list, "==",  TOKEN_DBL_EQ);
    reserved_token_list_append(punctuator_list, "=",   TOKEN_EQ);
    reserved_token_list_append(punctuator_list, "!=",  TOKEN_EXCL_EQ);
    reserved_token_list_append(punctuator_list, "!",   TOKEN_EXCL);
    reserved_token_list_append(punctuator_list, "<<=", TOKEN_DBL_LANGLE_EQ);
    reserved_token_list_append(punctuator_list, "<<",  TOKEN_DBL_LANGLE);
    reserved_token_list_append(punctuator_list, "<=",  TOKEN_LANGLE_EQ);
    reserved_token_list_append(punctuator_list, "<",   TOKEN_LANGLE);
    reserved_token_list_append(punctuator_list, ">>=", TOKEN_DBL_RANGLE_EQ);
    reserved_token_list_append(punctuator_list, ">>",  TOKEN_DBL_RANGLE);
    reserved_token_list_append(punctuator_list, ">=",  TOKEN_RANGLE_EQ);
    reserved_token_list_append(punctuator_list, ">",   TOKEN_RANGLE);
    reserved_token_list_append(punctuator_list, "++",  TOKEN_DBL_PLUS);
    reserved_token_list_append(punctuator_list, "+=",  TOKEN_PLUS_EQ);
    reserved_token_list_append(punctuator_list, "+",   TOKEN_PLUS);
    reserved_token_list_append(punctuator_list, "--",  TOKEN_DBL_MINUS);
    reserved_token_list_append(punctuator_list, "-=",  TOKEN_MINUS_EQ);
    reserved_token_list_append(punctuator_list, "-",   TOKEN_MINUS);
    reserved_token_list_append(punctuator_list, "*=",  TOKEN_ASTERISK_EQ);
    reserved_token_list_append(punctuator_list, "*",   TOKEN_ASTERISK);
    reserved_token_list_append(punctuator_list, "/=",  TOKEN_SLASH_EQ);
    reserved_token_list_append(punctuator_list, "/",   TOKEN_SLASH);
    reserved_token_list_append(punctuator_list, "%=",  TOKEN_PERCENT_EQ);
    reserved_token_list_append(punctuator_list, "%",   TOKEN_PERCENT);

    return punctuator_list;
}

ReservedTokenList* reserved_token_list_new_keywords() {
    size_t size = 8;
    ReservedTokenList* keyword_list = vector_new();
    vector_reserve(keyword_list, size);

    reserved_token_list_append(keyword_list, "break",    TOKEN_BREAK);
    reserved_token_list_append(keyword_list, "char",     TOKEN_CHAR);
    reserved_token_list_append(keyword_list, "continue", TOKEN_CONTINUE);
    reserved_token_list_append(keyword_list, "do",       TOKEN_DO);
    reserved_token_list_append(keyword_list, "else",     TOKEN_ELSE);
    reserved_token_list_append(keyword_list, "for",      TOKEN_FOR);
    reserved_token_list_append(keyword_list, "if",       TOKEN_IF);
    reserved_token_list_append(keyword_list, "int",      TOKEN_INT);
    reserved_token_list_append(keyword_list, "return",   TOKEN_RETURN);
    reserved_token_list_append(keyword_list, "while",    TOKEN_WHILE);

    return keyword_list;
}

ReservedTokenEntry* reserved_token_list_at(ReservedTokenList* token_list, size_t index) {
    return (ReservedTokenEntry*)vector_at(token_list, index);
}

void reserved_token_list_delete(ReservedTokenList* token_list) {
    if (token_list == NULL) return;

    size_t i = 0, size = token_list->size;
    for (i = 0; i < size; i++) {
        ReservedTokenEntry* entry = reserved_token_list_at(token_list, i);
        free(entry->value_token);
        free(entry);
        token_list->data[i] = NULL;
    }
    vector_delete(token_list);
}

void reserved_token_list_append(ReservedTokenList* token_list, char* value_token,  TokenType type) {
    ReservedTokenEntry* entry = (ReservedTokenEntry*)safe_malloc(sizeof(ReservedTokenEntry));
    entry->type = type;
    entry->value_token = str_new(value_token);
    vector_push_back(token_list, entry);
}
