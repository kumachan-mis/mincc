#include "localtable.h"

#include <stdio.h>
#include <stdlib.h>
#include "../common/memory.h"


// local-table-entry
LocalTableEntry* entry_new(CType* ctype, LocalSymbolStatus status);
void entry_delete(LocalTableEntry* entry);

// assertion
void local_redefinition_error(char* symbol_name);
void local_conflicting_type_error(char* symbol_name);


// local-table
LocalTable* local_table_new(LocalTable* parent) {
    LocalTable* local_table = (LocalTable*)safe_malloc(sizeof(LocalTable));
    local_table->inner_map = map_new();
    local_table->stack_offset = 0;
    local_table->parent = parent;
    return local_table;
}

void local_table_insert_copy(LocalTable* local_table, char* symbol_name, CType* ctype) {
    LocalTableEntry* existing_entry = (LocalTableEntry*)map_find(local_table->inner_map, symbol_name);
    if (existing_entry != NULL) {
        if (!ctype_equals(existing_entry->ctype, ctype)) local_conflicting_type_error(symbol_name);
        return;
    }
    LocalTableEntry* entry = entry_new(ctype_copy(ctype), LOCAL_SYMBOL_DECL);
    map_insert(local_table->inner_map, symbol_name, entry);
}

int local_table_exists(LocalTable* local_table, char* symbol_name) {
    if (local_table == NULL) return 0;

    LocalTableEntry* entry = (LocalTableEntry*)map_find(local_table->inner_map, symbol_name);
    if (entry != NULL) return 1;
    return local_table_exists(local_table->parent, symbol_name);
}

void local_table_define(LocalTable* local_table, char* symbol_name) {
    LocalTableEntry* entry = (LocalTableEntry*)map_find(local_table->inner_map, symbol_name);
    if (entry == NULL) return;
    if (entry->status == LOCAL_SYMBOL_DEF) {
        local_redefinition_error(symbol_name);
        return;
    }
    local_table->stack_offset += entry->ctype->size;
    entry->stack_index = local_table->stack_offset;
    entry->status = LOCAL_SYMBOL_DEF;
}

void local_table_enter_into_block_scope(LocalTable* block_table) {
    block_table->stack_offset = block_table->parent->stack_offset;
}

void local_table_exit_from_block_scope(LocalTable* block_table) {
    block_table->parent->stack_offset = block_table->stack_offset;
}

CType* local_table_get_ctype(LocalTable* local_table, char* symbol_name) {
    if (local_table == NULL) return NULL;

    LocalTableEntry* entry = (LocalTableEntry*)map_find(local_table->inner_map, symbol_name);
    if (entry != NULL) return entry->ctype;
    return local_table_get_ctype(local_table->parent, symbol_name);
}

int local_table_get_stack_index(LocalTable* local_table, char* symbol_name) {
    if (local_table == NULL) return -1;

    LocalTableEntry* entry = (LocalTableEntry*)map_find(local_table->inner_map, symbol_name);
    if (entry == NULL) return local_table_get_stack_index(local_table->parent, symbol_name);
    return entry->stack_index;
}

void local_table_delete(LocalTable* local_table) {
    if (local_table == NULL) return;

    Map* inner_map = local_table->inner_map;
    size_t i = 0, capacity = inner_map->capacity;
    for (i = 0; i < capacity; i++) {
        LocalTableEntry** entry_ptr = (LocalTableEntry**)(&inner_map->data[i].value);
        if (*entry_ptr == NULL) continue;
        entry_delete(*entry_ptr);
        *entry_ptr = NULL;
    }
    map_delete(local_table->inner_map);
    free(local_table);
}

// local-table-entry
LocalTableEntry* entry_new(CType* ctype, LocalSymbolStatus status) {
    LocalTableEntry* entry = (LocalTableEntry*)safe_malloc(sizeof(LocalTableEntry));
    entry->ctype = ctype;
    entry->stack_index = -1;
    entry->status = status;
    return entry;
}

void entry_delete(LocalTableEntry* entry) {
    if (entry == NULL) return;

    ctype_delete(entry->ctype);
    free(entry);
}

// assertion
void local_redefinition_error(char* symbol_name) {
    fprintf(stderr, "Error: redefinition of '%s'\n", symbol_name);
    exit(1);
}

void local_conflicting_type_error(char* symbol_name) {
    fprintf(stderr, "Error: conflicting type for '%s'\n", symbol_name);
    exit(1);
}
