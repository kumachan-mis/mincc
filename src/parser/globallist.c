#include "globallist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/memory.h"


// global-variable
GlobalVariable* global_variable_new(char* symbol_name, CType* ctype, GlobalSymbolStatus status);
void global_variable_delete(GlobalVariable* global_variable);

// util
GlobalVariable* global_list_find(GlobalList* global_list, char* symbol_name);

// assertion
void global_redefinition_error(char* symbol_name);
void global_conflicting_type_error(char* symbol_name);
void global_not_function_error(char* symbol_name);


// global-list
GlobalList* global_list_new() {
    return vector_new();
}

void global_list_insert_copy(GlobalList* global_list, char* symbol_name, CType* ctype) {
    GlobalVariable* existing_global_variable = global_list_find(global_list, symbol_name);
    if (existing_global_variable != NULL) {
        if (!ctype_equals(existing_global_variable->ctype, ctype)) global_conflicting_type_error(symbol_name);
        return;
    }

    symbol_name = str_new(symbol_name);
    ctype = ctype_copy(ctype);
    GlobalVariable* global_variable = global_variable_new(symbol_name, ctype, GLOBAL_SYMBOL_DECL);
    vector_push_back(global_list, global_variable);
}

int global_list_exists(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    return global_variable != NULL;
}

void global_list_tentatively_define(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable == NULL || global_variable->status != GLOBAL_SYMBOL_DECL) return;
    global_variable->global_data = global_data_new();
    global_data_set_zero_size(global_variable->global_data, global_variable->ctype->size);
    global_variable->status = GLOBAL_SYMBOL_TENT_DEF;
}

void global_list_define(GlobalList* global_list, char* symbol_name, GlobalData* global_data) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable == NULL) return;
    if (global_variable->status == GLOBAL_SYMBOL_DEF) {
        global_redefinition_error(symbol_name);
        return;
    }
    if (global_variable->status == GLOBAL_SYMBOL_TENT_DEF) {
        global_data_delete(global_variable->global_data);
    }
    global_variable->global_data = global_data;
    global_variable->status = GLOBAL_SYMBOL_DEF;
}

CType* global_list_get_ctype(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable != NULL) return global_variable->ctype;
    return NULL;
}

CType* global_list_get_function_ctype(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable != NULL) {
        if (global_variable->ctype->basic_ctype != CTYPE_FUNC) global_not_function_error(symbol_name);
        return global_variable->ctype;
    }
    fprintf(stderr, "Warning: no declaration of function '%s'\n", symbol_name);
    fprintf(stderr, "Warning: function type defaults to int %s()\n", symbol_name);
    symbol_name = str_new(symbol_name);
    CType* ctype = ctype_new_func(ctype_new_int(), vector_new());
    global_variable = global_variable_new(symbol_name, ctype, GLOBAL_SYMBOL_DECL);
    vector_push_back(global_list, ctype);
    return global_variable->ctype;
}

GlobalData* global_list_get_global_data(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable != NULL) return global_variable->global_data;
    return NULL;
}

void global_list_delete(GlobalList* global_list) {
    size_t i = 0, size = global_list->size;
    for (i = 0; i < size; i++) {
        global_variable_delete(vector_at(global_list, i));
        global_list->data[i] = NULL;
    }
    vector_delete(global_list);
}

// global-variable
GlobalVariable* global_variable_new(char* symbol_name, CType* ctype, GlobalSymbolStatus status) {
    GlobalVariable* global_variable = (GlobalVariable*)safe_malloc(sizeof(GlobalVariable));
    global_variable->symbol_name = symbol_name;
    global_variable->ctype = ctype;
    global_variable->global_data = NULL;
    global_variable->status = status;
    return global_variable;
}

void global_variable_delete(GlobalVariable* global_variable) {
    if (global_variable == NULL) return;

    free(global_variable->symbol_name);
    ctype_delete(global_variable->ctype);
    global_data_delete(global_variable->global_data);
    free(global_variable);
}

// util
GlobalVariable* global_list_find(GlobalList* global_list, char* symbol_name) {
    size_t i = 0, size = global_list->size;
    for (i = 0; i < size; i++) {
        GlobalVariable* global_variable = (GlobalVariable*)vector_at(global_list, i);
        if (strcmp(symbol_name, global_variable->symbol_name) == 0) return global_variable;
    }
    return NULL;
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
void global_redefinition_error(char* symbol_name) {
    fprintf(stderr, "Error: redefinition of '%s'\n", symbol_name);
    exit(1);
}

void global_conflicting_type_error(char* symbol_name) {
    fprintf(stderr, "Error: conflicting type for '%s'\n", symbol_name);
    exit(1);
}

void global_not_function_error(char* symbol_name) {
    fprintf(stderr, "Error: '%s' is not a function\n", symbol_name);
    exit(1);
}
