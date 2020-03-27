#include "lex.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"


// lex
Token* read_token(FILE* file_ptr);
Token* read_token_keyword_or_ident(FILE* file_ptr);
char* read_value_ident(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);
Token* read_token_punct(FILE* file_ptr);
void skip_spaces(FILE* file_ptr);

// token
TokenList* tokenlist_new();
Token* token_new(TokenType type);
void token_delete(Token* token);

// assertion
void assert_lexicon(int condition);


TokenList* tokenize(FILE* file_ptr) {
    TokenList* tokenlist = tokenlist_new();
    Vector* inner_vector = tokenlist->inner_vector;
    while (1) {
        skip_spaces(file_ptr);
        Token* token = read_token(file_ptr);
        vector_push_back(inner_vector, token);
        if (token->type == TOKEN_EOF) break;
    }
    return tokenlist;
}

// lex
Token* read_token(FILE* file_ptr) {
    char c = fgetc(file_ptr);
    ungetc(c, file_ptr);
    if (isalpha(c) || c == '_') {
        return read_token_keyword_or_ident(file_ptr);
    } else if (isdigit(c)) {
        return read_token_int(file_ptr);
    } else {
        return read_token_punct(file_ptr);
    }
}

Token* read_token_keyword_or_ident(FILE* file_ptr) {
    Token* token = NULL;
    char* value_ident = read_value_ident(file_ptr);
    if (strcmp(value_ident, "else") == 0) {
        token = token_new(TOKEN_ELSE);
    } else if (strcmp(value_ident, "if") == 0) {
        token = token_new(TOKEN_IF);
    } else if (strcmp(value_ident, "return") == 0) {
        token = token_new(TOKEN_RETURN);
    } else if (strcmp(value_ident, "while") == 0) {
        token = token_new(TOKEN_WHILE);
    } else {
        token = token_new(TOKEN_IDENT);
        token->value_ident = value_ident;
    }
    return token;
}

char* read_value_ident(FILE* file_ptr) {
    size_t len = 0, capacity = 2;
    char* value_ident = (char*)safe_malloc((capacity)*sizeof(char));

    char c = fgetc(file_ptr);
    assert_lexicon(isalpha(c) || c == '_');
    value_ident[len] = c;
    len++;

    while(1) {
        c = fgetc(file_ptr);
        if (!isalnum(c) && c != '_') {
            ungetc(c, file_ptr);
            break;
        }
        value_ident[len] = c;
        len++;
        if (len + 1 == capacity) {
            capacity *= 2;
            value_ident = (char*)safe_realloc(value_ident, (capacity)*sizeof(char));
        }
    }

    value_ident[len] = '\0';
    value_ident = (char*)safe_realloc(value_ident, (len+1)*sizeof(char));
    return value_ident;
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
    Token* token = token_new(TOKEN_INT_CONST);
    token->value_int = value;
    return token;
}

Token* read_token_punct(FILE* file_ptr) {
   char c = fgetc(file_ptr);

   switch (c) {
        case '|':
            c = fgetc(file_ptr);
            if (c == '|') return token_new(TOKEN_DBL_BAR);
            ungetc(c, file_ptr);
            return token_new(TOKEN_BAR);
        case '^':
            return token_new(TOKEN_HAT);
        case '&':
            c = fgetc(file_ptr);
            if (c == '&') return token_new(TOKEN_DBL_AND);
            ungetc(c, file_ptr);
            return token_new(TOKEN_AND);
        case '~':
            return token_new(TOKEN_TILDER);
        case '=':
            c = fgetc(file_ptr);
            if (c == '=') return token_new(TOKEN_DBL_EQ);
            ungetc(c, file_ptr);
            return token_new(TOKEN_EQ);
        case '!':
            c = fgetc(file_ptr);
            if (c == '=') return token_new(TOKEN_EXCL_EQ);
            ungetc(c, file_ptr);
            return token_new(TOKEN_EXCL);
        case '<':
            c = fgetc(file_ptr);
            if (c == '<') return token_new(TOKEN_DBL_LANGLE);
            if (c == '=') return token_new(TOKEN_LANGLE_EQ);
            ungetc(c, file_ptr);
            return token_new(TOKEN_LANGLE);
        case '>':
            c = fgetc(file_ptr);
            if (c == '>') return token_new(TOKEN_DBL_RANGLE);
            if (c == '=') return token_new(TOKEN_RANGLE_EQ);
            ungetc(c, file_ptr);
            return token_new(TOKEN_RANGLE);
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
        case ',':
            return token_new(TOKEN_COMMA);
        case ';':
            return token_new(TOKEN_SEMICOLON);
        case EOF:
            return token_new(TOKEN_EOF);
        default:
            assert_lexicon(0);
            return token_new(TOKEN_EOF);
    }
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

// token
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

Token* token_new(TokenType type) {
    Token* token = (Token*)safe_malloc(sizeof(Token));
    token->type = type;
    return token;
}

void token_delete(Token* token) {
    if (token->type == TOKEN_IDENT) {
        free(token->value_ident);
    }
    free(token);
}

// assertion
void assert_lexicon(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze lexicon\n");
    exit(1);
}
