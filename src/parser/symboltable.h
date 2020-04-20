#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_


#include "type.h"
#include "../common/map.h"
#include "../common/vector.h"


typedef struct {
    int size;
    int value_int;
    char* address_of;
} GlobalDatum;

typedef struct {
    Vector* data;
    int zero_size;
} GlobalData;

typedef enum {
    SYMBOL_DEF,
    SYMBOL_DECL
} SymbolStatus;

typedef struct {
    CType* ctype;
    GlobalData* global_data;
    int stack_index;
    SymbolStatus status;
} Entry;

typedef struct _SymbolTable {
    Map* inner_map;
    int stack_offset;
    struct _SymbolTable* parent;
} SymbolTable;


// symbol-table
SymbolTable* symbol_table_new(SymbolTable* parent);
void symbol_table_insert_copy(SymbolTable* symbol_table, char* symbol_name, CType* ctype);
int symbol_table_exists(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_define_global(SymbolTable* symbol_table, char* symbol_name, GlobalData* global_data);
void symbol_table_define_local(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_enter_into_block_scope(SymbolTable* block_table);
void symbol_table_exit_from_block_scope(SymbolTable* block_table);
CType* symbol_table_get_ctype(SymbolTable* symbol_table, char* symbol_name);
CType* symbol_table_get_function_ctype(SymbolTable* symbol_table, char* symbol_name);
GlobalData* symbol_table_get_global_data(SymbolTable* symbol_table, char* symbol_name);
int symbol_table_get_stack_index(SymbolTable* symbol_table, char* symbol_name);
void symbol_table_delete(SymbolTable* symbol_table);

// global-data
GlobalData* global_data_new();
void global_data_append_integer(GlobalData* global_data, int value_int, int size);
void global_data_append_address(GlobalData* global_data, char* address_of);
void global_data_set_zero_size(GlobalData* global_data, int zero_size);
void global_data_delete(GlobalData* global_data);


#endif
