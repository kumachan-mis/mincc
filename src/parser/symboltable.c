#include "symboltable.h"

#include <stdio.h>
#include <stdlib.h>
#include "../common/memory.h"


// assertion
void unbound_symbol_error();
void redefinition_error();


// symbol-table
SymbolTable* symbol_table_new() {
    SymbolTable* symbol_table = (SymbolTable*)safe_malloc(sizeof(SymbolTable));
    symbol_table->inner_map = map_new();
    symbol_table->stack_offset = 0;
    symbol_table->parent = NULL;
    return symbol_table;
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

// assertion
void unbound_symbol_error(char* symbol_name) {
    fprintf(stderr, "Error: unbound symbol %s\n", symbol_name);
    exit(1);
}
void redefinition_error(char* symbol_name) {
    fprintf(stderr, "Error: redefinition of %s\n", symbol_name);
    exit(1);
}
