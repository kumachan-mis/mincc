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

CType* ctype_new_int() {
    return ctype_new(CTYPE_INT, 8);
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

CType* ctype_copy(CType* ctype) {
    if (ctype == NULL) return NULL;
    CType* copied_ctype = ctype_new(ctype->basic_ctype, ctype->size);
    switch (copied_ctype->basic_ctype) {
        case CTYPE_INT:
            break;
        case CTYPE_PTR:
            copied_ctype->ptr_to = ctype_copy(ctype->ptr_to);
            break;
        case CTYPE_ARRAY:
            copied_ctype->array_of = ctype_copy(ctype->array_of);
            break;
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
        case CTYPE_INT:
            return 1;
        case CTYPE_PTR:
            return ctype_equals(ctype_x->ptr_to, ctype_y->ptr_to);
        case CTYPE_ARRAY:
            return ctype_x->size == ctype_y->size &&
                   ctype_equals(ctype_x->array_of, ctype_y->array_of);
        default:
            return 0;
    }
}

void ctype_delete(CType* ctype) {
    if (ctype == NULL) return;
    switch (ctype->basic_ctype) {
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
    }
    free(ctype);
}
