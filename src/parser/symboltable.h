#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_


#include "type.h"
#include "../common/map.h"


typedef enum {
    SYMBOL_DEF,
    SYMBOL_DECL
} SymbolStatus;

typedef struct {
    int stack_index;
    CType* ctype;
    SymbolStatus status;
} Entry;

typedef struct _SymbolTable {
    Map* inner_map;
    int stack_offset;
    struct _SymbolTable* parent;
} SymbolTable;


SymbolTable* symbol_table_new(SymbolTable* parent);
void symbol_table_insert_copy(
    SymbolTable* symbol_table,
    char* symbol_name, CType* ctype, SymbolStatus status);
CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name);
CType* symbol_table_get_function_ctype(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_push_into_stack(SymbolTable* symbol_table, char* symbol_name);
int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_delete(SymbolTable* symbol_table);


#endif
