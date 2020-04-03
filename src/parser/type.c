#include "type.h"

#include <stdlib.h>
#include <stddef.h>
#include "../common/memory.h"


// ctype
CType* ctype_new(BasicCType basic_ctype) {
    CType* ctype = (CType*)safe_malloc(sizeof(CType));
    ctype->basic_ctype = basic_ctype;
    ctype->ptr_to = NULL;
    return ctype;
}

CType* ctype_new_ptr(CType* ptr_to) {
    CType* ctype = ctype_new(CTYPE_PTR);
    ctype->ptr_to = ptr_to;
    return ctype;
}

CType* ctype_copy(CType* ctype) {
    if (ctype == NULL) return NULL;
    CType* copied_ctype = ctype_new(ctype->basic_ctype);
    copied_ctype->ptr_to = ctype_copy(ctype->ptr_to);
    return copied_ctype;
}

int ctype_size(CType* ctype) {
    /* implement later */
    return 8;
}

int ctype_equals(CType* ctype_x, CType* ctype_y) {
    if (ctype_x == NULL && ctype_y == NULL) return 1;
    if (ctype_x != NULL && ctype_y != NULL) {
        return ctype_x->basic_ctype == ctype_y->basic_ctype &&
               ctype_equals(ctype_x->ptr_to, ctype_y->ptr_to);
    }
    return 0;
}

void ctype_delete(CType* ctype) {
    if (ctype == NULL) return;
    ctype_delete(ctype->ptr_to);
    ctype->ptr_to = NULL;
    free(ctype);
}
