#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_


#include "type.h"
#include "../common/map.h"


typedef struct {
    int stack_index;
    CType* ctype;
} Entry;

typedef struct _SymbolTable {
    Map* inner_map;
    int stack_offset;
    struct _SymbolTable* parent;
} SymbolTable;


SymbolTable* symbol_table_new();
void symbol_table_enter_into_scope(SymbolTable* symbol_table, SymbolTable* parent);
void symbol_table_insert(SymbolTable* symbol_table, char* symbol_name, CType* ctype);
CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name, int is_callable);
int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_exit_from_scope(SymbolTable* symbol_table, SymbolTable* parent);
void symbol_table_delete(SymbolTable* symbol_table);


#endif