#ifndef _TYPE_H_
#define _TYPE_H_


typedef enum {
    CTYPE_INT,
    CTYPE_PTR,
    CTYPE_ARRAY,
} BasicCType;

typedef struct _CType {
    BasicCType basic_ctype;
    int size;
    union {
        struct _CType* ptr_to;
        struct _CType* array_of;
    };
} CType;


// ctype
CType* ctype_new_int();
CType* ctype_new_ptr(CType* ptr_to);
CType* ctype_new_array(CType* array_of, int len);
CType* ctype_copy(CType* ctype);
int ctype_equals(CType* ctype_x, CType* ctype_y);
void ctype_delete(CType* ctype);


#endif  // _TYPE_H_
