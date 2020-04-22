#ifndef _GLOBALLIST_H_
#define _GLOBALLIST_H_


#include "type.h"
#include "../common/vector.h"


typedef enum {
    GBL_TYPE_INTEGER,
    GBL_TYPE_ADDR,
    GBL_TYPE_STR
} GlobalDatumType;

typedef struct {
    GlobalDatumType type;
    int size;
    union {
        int value_int;
        char* address_of;
        char* value_str;
    };
} GlobalDatum;

typedef struct {
    Vector* inner_vector;
    int zero_size;
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
GlobalData* global_data_new();
void global_data_append_integer(GlobalData* global_data, int value_int, int size);
void global_data_append_address(GlobalData* global_data, char* address_of);
void global_data_append_string(GlobalData* global_data, char* value_str);
void global_data_set_zero_size(GlobalData* global_data, int zero_size);
void global_data_delete(GlobalData* global_data);


#endif  // _GLOBALLIST_H_
