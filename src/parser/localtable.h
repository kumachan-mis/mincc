#ifndef _LOCALTABLE_H_
#define _LOCALTABLE_H_


#include "type.h"
#include "../common/map.h"
#include "../common/vector.h"

typedef enum {
    LOCAL_SYMBOL_DEF,
    LOCAL_SYMBOL_DECL
} LocalSymbolStatus;

typedef struct {
    CType* ctype;
    int stack_index;
    LocalSymbolStatus status;
} LocalTableEntry;

typedef struct _LocalTable {
    Map* inner_map;
    int stack_offset;
    struct _LocalTable* parent;
} LocalTable;


// local-table
LocalTable* local_table_new(LocalTable* parent);
void local_table_insert_copy(LocalTable* local_table, char* symbol_name, CType* ctype);
int local_table_exists(LocalTable* local_table, char* symbol_name);
void local_table_define(LocalTable* local_table, char* symbol_name);
void local_table_enter_into_block_scope(LocalTable* block_table);
void local_table_exit_from_block_scope(LocalTable* block_table);
CType* local_table_get_ctype(LocalTable* local_table, char* symbol_name);
int local_table_get_stack_index(LocalTable* local_table, char* symbol_name);
void local_table_delete(LocalTable* local_table);


#endif  // _LOCALTABLE_H_
