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

void symbol_table_insert_copy(SymbolTable* symbol_table, char* symbol_name, CType* ctype) {
    Entry* existing_entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (existing_entry != NULL) {
        if (!ctype_equals(existing_entry->ctype, ctype)) conflicting_type_error(symbol_name);
        return;
    }
    Entry* entry = entry_new(ctype_copy(ctype), SYMBOL_DECL);
    map_insert(symbol_table->inner_map, str_new(symbol_name), entry);
}

int symbol_table_exists(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) return 0;
    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry != NULL) return 1;
    return symbol_table_exists(symbol_table->parent, symbol_name);
}

void symbol_table_define_global(SymbolTable* symbol_table, char* symbol_name, GlobalData* global_data) {
    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) {
        unbound_symbol_error(symbol_name);
        return;
    }
    if (entry->status == SYMBOL_DEF) {
        redefinition_error(symbol_name);
        return;
    }

    global_data_delete(entry->global_data);
    entry->global_data = global_data;
    if (global_data == NULL || entry->ctype->size > global_data->zero_size) {
        entry->status = SYMBOL_DEF;
    }
}

void symbol_table_define_local(SymbolTable* symbol_table, char* symbol_name) {
    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) {
        unbound_symbol_error(symbol_name);
        return;
    }
    if (entry->status == SYMBOL_DEF) {
        redefinition_error(symbol_name);
        return;
    }
    symbol_table->stack_offset += entry->ctype->size;
    entry->stack_index = symbol_table->stack_offset;
    entry->status = SYMBOL_DEF;
}

void symbol_table_enter_into_block_scope(SymbolTable* block_table) {
    block_table->stack_offset = block_table->parent->stack_offset;
}

void symbol_table_exit_from_block_scope(SymbolTable* block_table) {
    block_table->parent->stack_offset = block_table->stack_offset;
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


GlobalData* symbol_table_get_global_data(SymbolTable* symbol_table, char* symbol_name) {
    if (symbol_table == NULL) {
        unbound_symbol_error(symbol_name);
        return NULL;
    }

    Entry* entry = (Entry*)map_find(symbol_table->inner_map, symbol_name);
    if (entry == NULL) return symbol_table_get_global_data(symbol_table->parent, symbol_name);
    return entry->global_data;
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
    if (symbol_table == NULL) return;

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
    entry->ctype = ctype;
    entry->global_data = NULL;
    entry->stack_index = -1;
    entry->status = status;
    return entry;
}

void entry_delete(Entry* entry) {
    if (entry == NULL) return;

    ctype_delete(entry->ctype);
    global_data_delete(entry->global_data);
    free(entry);
}

// global-data
GlobalData* global_data_new() {
    GlobalData* global_data = (GlobalData*)safe_malloc(sizeof(GlobalData));
    global_data->data = vector_new();
    global_data->zero_size = 0;
    return global_data;
}

void global_data_append_integer(GlobalData* global_data, int value_int, int size) {
    GlobalDatum* datum = (GlobalDatum*)safe_malloc(sizeof(GlobalDatum));
    datum->size = size;
    datum->value_int = value_int;
    datum->address_of = NULL;
    vector_push_back(global_data->data, datum);
}

void global_data_append_address(GlobalData* global_data, char* address_of) {
    GlobalDatum* datum = (GlobalDatum*)safe_malloc(sizeof(GlobalDatum));
    datum->size = 8;
    datum->value_int = 0;
    datum->address_of = address_of;
    vector_push_back(global_data->data, datum);
}

void global_data_set_zero_size(GlobalData* global_data, int zero_size) {
    global_data->zero_size = zero_size;
}

void global_data_delete(GlobalData* global_data) {
     if (global_data == NULL) return;
    
    size_t i = 0, size = global_data->data->size;
    for (i = 0; i < size; i++) {
        GlobalDatum* datum = (GlobalDatum*)vector_at(global_data->data, i);
        free(datum->address_of);
        free(datum);
        global_data->data->data[i] = NULL;
    }
    vector_delete(global_data->data);
    free(global_data);
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
