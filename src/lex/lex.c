#include "lex.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fbuffer.h"
#include "../common/memory.h"


// lex
Token* read_token(FileBuffer* fbuffer, ReservedTokenList* keyword_list, ReservedTokenList* punct_list);
Token* read_token_keyword_or_ident(FileBuffer* fbuffer, ReservedTokenList* keyword_list);
Token* read_token_char(FileBuffer* fbuffer);
Token* read_token_int(FileBuffer* fbuffer);
Token* read_token_str(FileBuffer* fbuffer);
Token* read_token_punct(FileBuffer* fbuffer, ReservedTokenList* punct_list);
void skip_spaces(FileBuffer* fbuffer);

// utils
int read_value_char(FileBuffer* fbuffer);
int read_value_int(FileBuffer* fbuffer);
char* read_value_str(FileBuffer* fbuffer);
char* read_value_ident(FileBuffer* fbuffer);
int read_escape_sequence(FileBuffer* fbuffer);

// assertion
int assert_and_top_char(FileBuffer* fbuffer, int expected_char);
void assert_and_pop_char(FileBuffer* fbuffer, int expected_char);
void assert_lexicon(int condition);


TokenList* tokenize(FILE* file_ptr) {
    FileBuffer* fbuffer = fbuffer_new(file_ptr);
    ReservedTokenList* keyword_list = reserved_token_list_new_keywords();
    ReservedTokenList* punct_list = reserved_token_list_new_punctuators();

    TokenList* tokenlist = tokenlist_new();
    while (1) {
        skip_spaces(fbuffer);
        Token* token = read_token(fbuffer, keyword_list, punct_list);
        tokenlist_append(tokenlist, token);
        if (token->type == TOKEN_EOF) break;
    }

    reserved_token_list_delete(punct_list);
    reserved_token_list_delete(keyword_list);
    fbuffer_delete(fbuffer);

    return tokenlist;
}

// lex
Token* read_token(
    FileBuffer* fbuffer,
    ReservedTokenList* keyword_list,
    ReservedTokenList* punct_list
) {
    char c = fbuffer_top(fbuffer);
    if (isalpha(c) || c == '_') {
        return read_token_keyword_or_ident(fbuffer, keyword_list);
    } else if (c == '\'') {
        return read_token_char(fbuffer);
    } else if (isdigit(c)) {
        return read_token_int(fbuffer);
    } else if (c == '\"') {
        return read_token_str(fbuffer);
    } else {
        return read_token_punct(fbuffer, punct_list);
    }
}

Token* read_token_keyword_or_ident(FileBuffer* fbuffer, ReservedTokenList* keyword_list) {
    char* value_ident = read_value_ident(fbuffer);
    size_t i = 0, size = keyword_list->size;
    for (i = 0; i < size; i++) {
        ReservedTokenEntry* entry = reserved_token_list_at(keyword_list, i);
        if (strcmp(value_ident, entry->value_token) == 0) {
            free(value_ident);
            return token_new(entry->type);
        }
    }
    return token_new_ident(TOKEN_IDENT, value_ident);
}

Token* read_token_char(FileBuffer* fbuffer) {
    // character constant has type int
    int value_char = read_value_char(fbuffer);
    return token_new_int(TOKEN_IMM_INT, value_char);
}

Token* read_token_int(FileBuffer* fbuffer) {
    int value_int = read_value_int(fbuffer);
    return token_new_int(TOKEN_IMM_INT, value_int);
}

Token* read_token_str(FileBuffer* fbuffer) {
    char* value_str = read_value_str(fbuffer);
    return token_new_str(TOKEN_IMM_STR, value_str);
}

Token* read_token_punct(FileBuffer* fbuffer, ReservedTokenList* punct_list) {
    if (fbuffer_top(fbuffer) == EOF) return token_new(TOKEN_EOF);

    size_t i = 0, size = punct_list->size;
    for (i = 0; i < size; i++) {
        ReservedTokenEntry* entry = reserved_token_list_at(punct_list, i);
        if (fbuffer_starts_with(fbuffer, entry->value_token)) {
            fbuffer_popn(fbuffer, strlen(entry->value_token));
            return token_new(entry->type);
        }
    }
    return token_new(TOKEN_EOF);
}

void skip_spaces(FileBuffer* fbuffer) {
    while (1) {
        char c = fbuffer_top(fbuffer);
        if (!isspace(c)) break;
        fbuffer_pop(fbuffer);
    }
}

// utils
int read_value_char(FileBuffer* fbuffer) {
    // character constant has type int
    assert_and_pop_char(fbuffer, '\'');
    int value_char = fbuffer_top(fbuffer);
    if (value_char != '\\') fbuffer_pop(fbuffer);
    else                    value_char = read_escape_sequence(fbuffer);
    assert_and_pop_char(fbuffer, '\'');
    return value_char;
}

int read_value_int(FileBuffer* fbuffer) {
    int value_int = 0;
    while (1) {
        char c = fbuffer_top(fbuffer);
        if (!isdigit(c)) break;
        value_int = 10 * value_int + (c - '0');
        fbuffer_pop(fbuffer);
    }
    return value_int;
}

char* read_value_str(FileBuffer* fbuffer) {
    assert_and_pop_char(fbuffer, '\"');
    size_t len = 0, capacity = 2;
    char* value_str = (char*)safe_malloc((capacity)*sizeof(char));

    while(1) {
        char c = fbuffer_top(fbuffer);
        if (c == '\"') {
            fbuffer_pop(fbuffer);
            break;
        }
        if (c == '\\') c = read_escape_sequence(fbuffer);
        else           fbuffer_pop(fbuffer);
        value_str[len] = c; len++;
        if (capacity <= len + 1) {
            capacity *= 2;
            value_str = (char*)safe_realloc(value_str, (capacity)*sizeof(char));
        }
    }

    value_str[len] = '\0';
    value_str = (char*)safe_realloc(value_str, (len+1)*sizeof(char));
    return value_str;
}

char* read_value_ident(FileBuffer* fbuffer) {
    size_t len = 0, capacity = 2;
    char* value_ident = (char*)safe_malloc((capacity)*sizeof(char));

    char c = fbuffer_top(fbuffer);
    assert_lexicon(isalpha(c) || c == '_');
    value_ident[len] = c; len++;
    fbuffer_pop(fbuffer);

    while(1) {
        c = fbuffer_top(fbuffer);
        if (!isalnum(c) && c != '_') break;
        value_ident[len] = c; len++;
        fbuffer_pop(fbuffer);
        if (capacity <= len + 1) {
            capacity *= 2;
            value_ident = (char*)safe_realloc(value_ident, (capacity)*sizeof(char));
        }
    }

    value_ident[len] = '\0';
    value_ident = (char*)safe_realloc(value_ident, (len+1)*sizeof(char));
    return value_ident;
}

int read_escape_sequence(FileBuffer* fbuffer) {
    assert_and_pop_char(fbuffer, '\\');
    int value_int = fbuffer_top(fbuffer);
    fbuffer_pop(fbuffer);
    switch (value_int) {
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '\?':
            return '\?';
        case '\\':
            return '\\';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case 'v':
            return '\v';
        case '0':
            return '\0';
        default:
            assert_lexicon(0);
            return '\0';
    }
}

// assertion
int assert_and_top_char(FileBuffer* fbuffer, int expected_char) {
    char c = fbuffer_top(fbuffer);
    assert_lexicon(c == expected_char);
    return c;
}

void assert_and_pop_char(FileBuffer* fbuffer, int expected_char) {
    char c = fbuffer_top(fbuffer);
    assert_lexicon(c == expected_char);
    fbuffer_pop(fbuffer);
}

void assert_lexicon(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze lexicon\n");
    exit(1);
}
