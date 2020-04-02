#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_


#include "map.h"


typedef enum {
    CTYPE_INT,
    CTYPE_PTR
} BasicCType;

typedef struct _CType {
    BasicCType basic_ctype;
    struct _CType* ptr_to;
} CType;

typedef struct {
    int stack_index;
    CType* ctype;
} Entry;

typedef struct _SymbolTable {
    Map* inner_map;
    int stack_offset;
    struct _SymbolTable* parent;
} SymbolTable;


// symbol-table
SymbolTable* symbol_table_new();
void symbol_table_enter_into_scope(SymbolTable* symbol_table, SymbolTable* parent);
void symbol_table_insert(SymbolTable* symbol_table, char* symbol_name, CType* ctype);
CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name);
int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_exit_from_scope(SymbolTable* symbol_table, SymbolTable* parent);
void symbol_table_delete(SymbolTable* symbol_table);

// ctype
CType* ctype_new(BasicCType basic_ctype);
CType* ctype_new_ptr(CType* ptr_to);
CType* ctype_copy(CType* ctype);
int ctype_size(CType* ctype);
int ctype_equals(CType* ctype_x, CType* ctype_y);
void ctype_delete(CType* ctype);


#endif
