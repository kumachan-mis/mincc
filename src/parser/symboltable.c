#include "symboltable.h"

#include <stdio.h>
#include <stdlib.h>
#include "../common/memory.h"


// entry
Entry* entry_new(CType* ctype, SymbolStatus status);
void entry_delete(Entry* entry);

// assertion
void unbound_symbol_error(char* symbol_name);
void redefinition_error(char* symbol_name);
void conflicting_type_error(char* symbol_name);
void assert_function_type(CType* ctype);


// symbol-table
SymbolTable* symbol_table_new(SymbolTable* parent) {
    SymbolTable* symbol_table = (SymbolTable*)safe_malloc(sizeof(SymbolTable));
    symbol_table->inner_map = map_new();
    symbol_table->stack_offset = 0;
    symbol_table->parent = parent;
    return symbol_table;
}

void symbol_table_insert_copy(
    SymbolTable* symbol_table,
    char* symbol_name, CType* ctype, SymbolStatus status
) {
    Entry* existing_entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (existing_entry != NULL) {
        if (!ctype_equals(existing_entry->ctype, ctype)) conflicting_type_error(symbol_name);

        if (existing_entry->status == SYMBOL_DECL && status == SYMBOL_DEF) {
            existing_entry->status = SYMBOL_DEF;
        } else if (existing_entry->status == SYMBOL_DEF && status == SYMBOL_DEF) {
            redefinition_error(symbol_name);
        }
    } else {
        Entry* entry = entry_new(ctype_copy(ctype), status);
        map_insert(symbol_table->inner_map, str_new(symbol_name), entry);
    }
}

CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return NULL;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry != NULL) return entry->ctype;
    return symbol_table_get_ctype(symbol_table->parent, symbol_name);
}

CType* symbol_table_get_function_ctype(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return NULL;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry != NULL) {
        assert_function_type(entry->ctype);
        return entry->ctype;
    }
    if (symbol_table->parent == NULL) {
        fprintf(stderr, "Warning: no declaration of function '%s'\n", symbol_name);
        fprintf(stderr, "Warning: return type defaults to int\n");
        CType* ctype = ctype_new_func(ctype_new_int(), vector_new());
        map_insert(symbol_table->inner_map, str_new(symbol_name), entry_new(ctype, SYMBOL_DECL));
        return ctype;
    }
    return symbol_table_get_function_ctype(symbol_table->parent, symbol_name);
}

void symbol_table_push_into_stack(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return;
    }
    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) {
        unbound_symbol_error(symbol_name);
        return;
    }
    
    symbol_table->stack_offset += entry->ctype->size;
    entry->stack_index = symbol_table->stack_offset;
}

int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return -1;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) return symbol_table_get_stack_index(symbol_table->parent, symbol_name);
    return entry->stack_index;
}

void symbol_table_delete(SymbolTable* symbol_table) {
    Map* inner_map = symbol_table->inner_map;
    size_t i = 0, capacity = inner_map->capacity;
    for (i = 0; i < capacity; i++) {
        Entry** entry_ptr = (Entry**)(&inner_map->data[i].value);
        if (*entry_ptr == NULL) continue;
        entry_delete(*entry_ptr);
        *entry_ptr = NULL;
    }
    map_delete(symbol_table->inner_map);
    free(symbol_table);
}

// entry
Entry* entry_new(CType* ctype, SymbolStatus status) {
    Entry* entry = (Entry*)safe_malloc(sizeof(Entry));
    entry->stack_index = -1;
    entry->ctype = ctype;
    entry->status = status;
    return entry;
}

void entry_delete(Entry* entry) {
    ctype_delete(entry->ctype);
    free(entry);
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

void conflicting_type_error(char* symbol_name) {
    fprintf(stderr, "Error: conflicting type for '%s'\n", symbol_name);
    exit(1);
}

void assert_function_type(CType* ctype) {
    if (ctype->basic_ctype == CTYPE_FUNC) return;
    fprintf(stderr, "Error: not function type\n");
    exit(1);
}
