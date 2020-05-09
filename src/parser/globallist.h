#ifndef _GLOBALLIST_H_
#define _GLOBALLIST_H_


#include "type.h"
#include "../common/vector.h"


typedef enum {
    GBL_TYPE_INTEGER,
    GBL_TYPE_ADDR,
    GBL_TYPE_STR,
    GBL_TYPE_LIST
} GlobalDataType;

typedef struct {
    GlobalDataType type;
    int size;
    union {
        int value_int;
        char* address_of;
        char* value_str;
        Vector* children;
    };
} GlobalData;

typedef enum {
    GLOBAL_SYMBOL_DEF,
    GLOBAL_SYMBOL_TENT_DEF,
    GLOBAL_SYMBOL_DECL
} GlobalSymbolStatus;

typedef struct {
    char* symbol_name;
    CType* ctype;
    GlobalData* global_data;
    GlobalSymbolStatus status;
} GlobalVariable;

typedef struct {
     Vector* inner_vector;
     int num_str_labels;
     int pos;
} GlobalList;


// global-list
GlobalList* global_list_new();
void global_list_insert_copy(GlobalList* global_list, char* symbol_name, CType* ctype);
GlobalVariable* global_list_top(GlobalList* global_list);
void global_list_pop(GlobalList* global_list);
int global_list_exists(GlobalList* global_list, char* symbol_name);
void global_list_tentatively_define(GlobalList* global_list, char* symbol_name);
void global_list_define(GlobalList* global_list, char* symbol_name, GlobalData* global_data);
CType* global_list_get_ctype(GlobalList* global_list, char* symbol_name);
CType* global_list_get_function_ctype(GlobalList* global_list, char* symbol_name);
GlobalData* global_list_get_global_data(GlobalList* global_list, char* symbol_name);
char* global_list_create_str_label(GlobalList* global_list);
void global_list_delete(GlobalList* global_list);

// global-data
GlobalData* global_data_new_list();
GlobalData* global_data_new_integer(int value_int, int size);
GlobalData* global_data_new_address(char* address_of);
GlobalData* global_data_new_string(char* value_str);
void global_data_append_child(GlobalData* global_data, GlobalData* child);
GlobalData* global_data_nth_child(GlobalData* global_data, size_t n);
void global_data_delete(GlobalData* global_data);


#endif  // _GLOBALLIST_H_
