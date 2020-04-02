#include "symboltable.h"

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"


// assertion
void unbound_symbol_error();
void redefinition_error();


// symbol-table
SymbolTable* symbol_table_new() {
    SymbolTable* symbol_table = (SymbolTable*)safe_malloc(sizeof(SymbolTable));
    symbol_table->inner_map = map_new();
    symbol_table->stack_offset = 0;
    symbol_table->parent = NULL;
}

void symbol_table_enter_into_scope(SymbolTable* symbol_table, SymbolTable* parent) {
    symbol_table->parent = parent;
    if (parent != NULL) {
        symbol_table->stack_offset = parent->stack_offset;
    }
}

void symbol_table_insert(SymbolTable* symbol_table, char* symbol_name, CType* ctype) {
    if (map_find(symbol_table->inner_map, symbol_name) != NULL) {
        redefinition_error(symbol_name);
        return;
    }

    symbol_table->stack_offset += ctype_size(ctype);
    Entry* entry = (Entry*)safe_malloc(sizeof(Entry));
    entry->ctype = ctype;
    entry->stack_index = symbol_table->stack_offset;
    map_insert(symbol_table->inner_map, symbol_name, entry);
}

CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        // unbound_symbol_error(symbol_name);
        return NULL;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) return symbol_table_get_ctype(symbol_table->parent, symbol_name);
    return entry->ctype;
}

int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return 0;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) return symbol_table_get_stack_index(symbol_table->parent, symbol_name);
    return entry->stack_index;
}

void symbol_table_exit_from_scope(SymbolTable* symbol_table, SymbolTable* parent) {
    if (parent != NULL) {
        parent->stack_offset = symbol_table->stack_offset;
    }
}

void symbol_table_delete(SymbolTable* symbol_table) {
    map_delete(symbol_table->inner_map);
    free(symbol_table);
}


// ctype
CType* ctype_new(BasicCType basic_ctype) {
    CType* ctype = (CType*)safe_malloc(sizeof(CType));
    ctype->basic_ctype = basic_ctype;
    ctype->ptr_to = NULL;
    return ctype;
}

CType* ctype_new_ptr(CType* ptr_to) {
    CType* ctype = ctype_new(CTYPE_PTR);
    ctype->ptr_to = ptr_to;
    return ctype;
}

CType* ctype_copy(CType* ctype) {
    if (ctype == NULL) return NULL;
    CType* copied_ctype = ctype_new(ctype->basic_ctype);
    copied_ctype->ptr_to = ctype_copy(ctype->ptr_to);
    return copied_ctype;
}

int ctype_size(CType* ctype) {
    /* implement later */
    return 8;
}

int ctype_equals(CType* ctype_x, CType* ctype_y) {
    if (ctype_x == NULL && ctype_y == NULL) return 1;
    if (ctype_x != NULL && ctype_y != NULL) {
        return ctype_x->basic_ctype == ctype_y->basic_ctype &&
               ctype_equals(ctype_x->ptr_to, ctype_y->ptr_to);
    }
    return 0;
}

void ctype_delete(CType* ctype) {
    if (ctype == NULL) return;
    ctype_delete(ctype->ptr_to);
    ctype->ptr_to = NULL;
    free(ctype);
}

// assertion
void unbound_symbol_error(char* symbol_name) {
    fprintf(stderr, "Error: unbound symbol %s\n", symbol_name);
    exit(1);
}
void redefinition_error(char* symbol_name) {
    fprintf(stderr, "Error: redefinition of %s\n", symbol_name);
    exit(1);
}
