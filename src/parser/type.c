#include "type.h"

#include <stdlib.h>
#include <stddef.h>
#include "../common/memory.h"


// ctype
CType* ctype_new(BasicCType basic_ctype, int size) {
    CType* ctype = (CType*)safe_malloc(sizeof(CType));
    ctype->basic_ctype = basic_ctype;
    ctype->size = size;
    return ctype;
}

CType* ctype_new_char() {
    return ctype_new(CTYPE_CHAR, 1);
}

CType* ctype_new_int() {
    return ctype_new(CTYPE_INT, 4);
}

CType* ctype_new_ptr(CType* ptr_to) {
    CType* ctype = ctype_new(CTYPE_PTR, 8);
    ctype->ptr_to = ptr_to;
    return ctype;
}

CType* ctype_new_array(CType* array_of, int len) {
    CType* ctype = ctype_new(CTYPE_ARRAY, array_of->size * len);
    ctype->array_of = array_of;
    return ctype;
}

CType* ctype_new_func(CType* return_type, Vector* param_types) {
    CType* ctype = ctype_new(CTYPE_FUNC, 0);
    ctype->func = (CFuncType*)safe_malloc(sizeof(CFuncType));
    ctype->func->return_type = return_type;
    ctype->func->param_types = param_types;
    return ctype;
}

CType* ctype_copy(CType* ctype) {
    if (ctype == NULL) return NULL;

    CType* copied_ctype = ctype_new(ctype->basic_ctype, ctype->size);
    switch (copied_ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
            break;
        case CTYPE_PTR:
            copied_ctype->ptr_to = ctype_copy(ctype->ptr_to);
            break;
        case CTYPE_ARRAY:
            copied_ctype->array_of = ctype_copy(ctype->array_of);
            break;
        case CTYPE_FUNC: {
            CFuncType* func = ctype->func;
            CFuncType* copied_func = (CFuncType*)safe_malloc(sizeof(CFuncType));
            copied_func->return_type = ctype_copy(func->return_type);
            copied_func->param_types = vector_new();
            size_t i = 0, size = func->param_types->size;
            vector_reserve(copied_func->param_types, size);
            for (i = 0; i < size; i++) {
                CType* param_type = vector_at(func->param_types, i);
                vector_push_back(copied_func->param_types, ctype_copy(param_type));
            }
            copied_ctype->func = copied_func;
            break;
        }
    }
    return copied_ctype;
}

int ctype_equals(CType* ctype_x, CType* ctype_y) {
    if (ctype_x == NULL && ctype_y == NULL) return 1;
    if (ctype_x == NULL || ctype_y == NULL) return 0;

    if (ctype_x->basic_ctype != ctype_y->basic_ctype) {
        return 0;
    }
    switch (ctype_x->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
            return 1;
        case CTYPE_PTR:
            return ctype_equals(ctype_x->ptr_to, ctype_y->ptr_to);
        case CTYPE_ARRAY:
            return ctype_x->size == ctype_y->size &&
                   ctype_equals(ctype_x->array_of, ctype_y->array_of);
        case CTYPE_FUNC: {
            CFuncType* func_x = ctype_x->func;
            CFuncType* func_y = ctype_y->func;
            if (func_x->param_types->size != func_y->param_types->size) {
                return 0;
            }
            if (!ctype_equals(func_x->return_type, func_y->return_type)) {
                return 0;
            }
            size_t i = 0, size = func_x->param_types->size;
            for (i = 0; i < size; i++) {
                CType* param_type_x = vector_at(func_x->param_types, i);
                CType* param_type_y = vector_at(func_y->param_types, i);
                if (!ctype_equals(param_type_x, param_type_y)) {
                    return 0;
                }
            }
            return 1;
        }
        default:
            return 0;
    }
}

int ctype_compatible(CType* ctype_x, CType* ctype_y) {
    return (ctype_is_integer_ctype(ctype_x)   && ctype_is_integer_ctype(ctype_y))  ||
           (ctype_x->basic_ctype == CTYPE_PTR && ctype_y->basic_ctype == CTYPE_PTR);
}

void ctype_delete(CType* ctype) {
    if (ctype == NULL) return;

    switch (ctype->basic_ctype) {
        case CTYPE_CHAR:
        case CTYPE_INT:
            break;
        case CTYPE_PTR:
            ctype_delete(ctype->ptr_to);
            ctype->ptr_to = NULL;
            break;
        case CTYPE_ARRAY:
            ctype_delete(ctype->array_of);
            ctype->array_of = NULL;
            break;
        case CTYPE_FUNC: {
            CFuncType* func = ctype->func;
            ctype_delete(func->return_type);
            size_t i = 0, size = func->param_types->size;
            for (i = 0; i < size; i++) {
                ctype_delete(vector_at(func->param_types, i));
                func->param_types->data[i] = NULL;
            }
            vector_delete(func->param_types);
            free(func);
            break;
        }
    }
    free(ctype);
}

// ctype-classifier
int ctype_is_integer_ctype(CType* ctype) {
    BasicCType basic_ctype = ctype->basic_ctype;
    return basic_ctype == CTYPE_INT || basic_ctype == CTYPE_CHAR;
}
