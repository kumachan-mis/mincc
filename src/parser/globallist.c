#include "globallist.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/memory.h"


// global-list
GlobalVariable* global_list_find(GlobalList* global_list, char* symbol_name);

// global-variable
GlobalVariable* global_variable_new(char* symbol_name, CType* ctype, GlobalSymbolStatus status);
void global_variable_delete(GlobalVariable* global_variable);

// assertion
void global_redefinition_error(char* symbol_name);
void global_conflicting_type_error(char* symbol_name);
void global_not_function_error(char* symbol_name);
void str_label_limit_error();


// global-list
GlobalList* global_list_new() {
    GlobalList* global_list = (GlobalList*)safe_malloc(sizeof(GlobalList));
    global_list->inner_vector = vector_new();
    global_list->num_str_labels = 0;
    global_list->pos = 0;
    return global_list;
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
    vector_push_back(global_list->inner_vector, global_variable);
}

GlobalVariable* global_list_top(GlobalList* global_list) {
    return (GlobalVariable*)vector_at(global_list->inner_vector, global_list->pos);
}

void global_list_pop(GlobalList* global_list) {
    global_list->pos++;
}

GlobalVariable* global_list_find(GlobalList* global_list, char* symbol_name) {
    Vector* inner_vector = global_list->inner_vector;
    size_t i = 0, size = inner_vector->size;
    for (i = 0; i < size; i++) {
        GlobalVariable* global_variable = (GlobalVariable*)vector_at(inner_vector, i);
        if (strcmp(symbol_name, global_variable->symbol_name) == 0) return global_variable;
    }
    return NULL;
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
    vector_push_back(global_list->inner_vector, ctype);
    return global_variable->ctype;
}

GlobalData* global_list_get_global_data(GlobalList* global_list, char* symbol_name) {
    GlobalVariable* global_variable = global_list_find(global_list, symbol_name);
    if (global_variable != NULL) return global_variable->global_data;
    return NULL;
}

char* global_list_create_str_label(GlobalList* global_list) {
    if (global_list->num_str_labels == 1 << 30) {
        str_label_limit_error();
        return NULL;
    }
    char* label = safe_malloc(14 * sizeof(char));
    sprintf(label, ".LC%d", global_list->num_str_labels);
    global_list->num_str_labels++;
    return label;
}

void global_list_delete(GlobalList* global_list) {
    Vector* inner_vector = global_list->inner_vector;
    size_t i = 0, size = inner_vector->size;
    for (i = 0; i < size; i++) {
        global_variable_delete(vector_at(inner_vector, i));
        inner_vector->data[i] = NULL;
    }
    vector_delete(inner_vector);
    free(global_list);
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

// global-data
GlobalData* global_data_new() {
    GlobalData* global_data = (GlobalData*)safe_malloc(sizeof(GlobalData));
    global_data->inner_vector = vector_new();
    global_data->zero_size = 0;
    return global_data;
}

void global_data_append_integer(GlobalData* global_data, int value_int, int size) {
    GlobalDatum* datum = (GlobalDatum*)safe_malloc(sizeof(GlobalDatum));
    datum->type = GBL_TYPE_INTEGER;
    datum->size = size;
    datum->value_int = value_int;
    vector_push_back(global_data->inner_vector, datum);
}

void global_data_append_address(GlobalData* global_data, char* address_of) {
    GlobalDatum* datum = (GlobalDatum*)safe_malloc(sizeof(GlobalDatum));
    datum->type = GBL_TYPE_ADDR;
    datum->size = 8;
    datum->address_of = address_of;
    vector_push_back(global_data->inner_vector, datum);
}

void global_data_append_string(GlobalData* global_data, char* value_str) {
    GlobalDatum* datum = (GlobalDatum*)safe_malloc(sizeof(GlobalDatum));
    datum->type = GBL_TYPE_STR;
    datum->size = (strlen(value_str)+1) * sizeof(char);
    datum->value_str = value_str;
    vector_push_back(global_data->inner_vector, datum);
}

GlobalDatum* global_data_nth_datum(GlobalData* global_data, size_t n) {
    return (GlobalDatum*)vector_at(global_data->inner_vector, n);
}

void global_data_set_zero_size(GlobalData* global_data, int zero_size) {
    global_data->zero_size = zero_size;
}

void global_data_delete(GlobalData* global_data) {
     if (global_data == NULL) return;
    
    size_t i = 0, size = global_data->inner_vector->size;
    for (i = 0; i < size; i++) {
        GlobalDatum* datum = (GlobalDatum*)vector_at(global_data->inner_vector, i);
        switch (datum->type) {
            case GBL_TYPE_INTEGER:
                break;
            case GBL_TYPE_ADDR:
                free(datum->address_of);
                break;
            case GBL_TYPE_STR:
                free(datum->value_str);
                break;
        }
        free(datum);
        global_data->inner_vector->data[i] = NULL;
    }
    vector_delete(global_data->inner_vector);
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

void str_label_limit_error() {
    fprintf(stderr, "Error: cannot create new label\n");
    exit(1);
}
