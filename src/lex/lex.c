#include "lex.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../common/memory.h"


// lex
Token* read_token(FILE* file_ptr);
Token* read_token_keyword_or_ident(FILE* file_ptr);
char* read_value_ident(FILE* file_ptr);
Token* read_token_int(FILE* file_ptr);
Token* read_token_punct(FILE* file_ptr);
void skip_spaces(FILE* file_ptr);

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
    char* value_ident = read_value_ident(file_ptr);

    if (strcmp(value_ident, "do") == 0)          return token_new(TOKEN_DO);
    else if (strcmp(value_ident, "else") == 0)   return token_new(TOKEN_ELSE);
    else if (strcmp(value_ident, "for") == 0)    return token_new(TOKEN_FOR);
    else if (strcmp(value_ident, "if") == 0)     return token_new(TOKEN_IF);
    else if (strcmp(value_ident, "int") == 0)    return token_new(TOKEN_INT);
    else if (strcmp(value_ident, "return") == 0) return token_new(TOKEN_RETURN);
    else if (strcmp(value_ident, "while") == 0)  return token_new(TOKEN_WHILE);
    return token_new_ident(TOKEN_IDENT, value_ident);
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
    int value_int = 0;
    while (1) {
        char c = fgetc(file_ptr);
        if (!isdigit(c)) {
            ungetc(c, file_ptr);
            break;
        }
        value_int = 10 * value_int + (c - '0');
    }
    return token_new_int(TOKEN_IMM_INT, value_int);;
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
        case '{':
            return token_new(TOKEN_LBRACE);
        case '}':
            return token_new(TOKEN_RBRACE);
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

// assertion
void assert_lexicon(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze lexicon\n");
    exit(1);
}
