#include "lex.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "fbuffer.h"
#include "../common/memory.h"


// lex
Token* read_token(
    FileBuffer* fbuffer,
    ReservedTokenList* keyword_list,
    ReservedTokenList* punct_list
);
Token* read_token_keyword_or_ident(FileBuffer* fbuffer, ReservedTokenList* keyword_list);
Token* read_token_int(FileBuffer* fbuffer);
Token* read_token_punct(FileBuffer* fbuffer, ReservedTokenList* punct_list);
void skip_spaces(FileBuffer* fbuffer);
char* read_value_ident(FileBuffer* fbuffer);

// assertion
void assert_lexicon(int condition);


TokenList* tokenize(FILE* file_ptr) {
    FileBuffer* fbuffer = fbuffer_new(file_ptr);
    ReservedTokenList* keyword_list = reserved_token_list_new_keywords();
    ReservedTokenList* punct_list = reserved_token_list_new_punctuators();

    TokenList* tokenlist = tokenlist_new();
    Vector* inner_vector = tokenlist->inner_vector;
    while (1) {
        skip_spaces(fbuffer);
        Token* token = read_token(fbuffer, keyword_list, punct_list);
        vector_push_back(inner_vector, token);
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
    } else {
        return read_token_punct(fbuffer, punct_list);
    }
}

Token* read_token_keyword_or_ident(FileBuffer* fbuffer, ReservedTokenList* keyword_list) {
    char* value_ident = read_value_ident(fbuffer);
    size_t i = 0, size = keyword_list->size;
    for (i = 0; i < size; i++) {
        ReservedTokenEntry* entry = reserved_token_list_at(keyword_list, i);
        if (strcmp(value_ident, entry->value_token) == 0) return token_new(entry->type);
    }
    return token_new_ident(TOKEN_IDENT, value_ident);
}

Token* read_token_char(FileBuffer* fbuffer) {
    // character constant has type int
   assert_and_pop_char(fbuffer, '\'');
   int value_int = fbuffer_top(fbuffer);
   fbuffer_pop(fbuffer);
   assert_and_pop_char(fbuffer, '\'');
   return token_new_int(TOKEN_IMM_INT, value_int);
}

Token* read_token_int(FileBuffer* fbuffer) {
    int value_int = 0;
    while (1) {
        char c = fbuffer_top(fbuffer);
        if (!isdigit(c)) break;
        value_int = 10 * value_int + (c - '0');
        fbuffer_pop(fbuffer);
    }
    return token_new_int(TOKEN_IMM_INT, value_int);;
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
        if (len + 1 == capacity) {
            capacity *= 2;
            value_ident = (char*)safe_realloc(value_ident, (capacity)*sizeof(char));
        }
    }

    value_ident[len] = '\0';
    value_ident = (char*)safe_realloc(value_ident, (len+1)*sizeof(char));
    return value_ident;
}

// assertion
void assert_lexicon(int condition) {
    if (condition) return;
    fprintf(stderr, "Error: fail to analyze lexicon\n");
    exit(1);
}
