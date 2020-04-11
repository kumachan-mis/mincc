#ifndef _TYPE_H_
#define _TYPE_H_


typedef enum {
    CTYPE_INT,
    CTYPE_PTR
} BasicCType;

typedef struct _CType {
    BasicCType basic_ctype;
    struct _CType* ptr_to;
} CType;


// ctype
CType* ctype_new(BasicCType basic_ctype);
CType* ctype_new_ptr(CType* ptr_to);
CType* ctype_copy(CType* ctype);
int ctype_size(CType* ctype);
int ctype_equals(CType* ctype_x, CType* ctype_y);
void ctype_delete(CType* ctype);


#endif  // _TYPE_H_
