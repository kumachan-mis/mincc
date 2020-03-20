#ifndef _MINCC_VECTOR_H_
#define _MINCC_VECTOR_H_


#include <stddef.h>


typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} Vector;

Vector* vector_new();
void vector_reserve(Vector* vector, size_t size);
void vector_push_back(Vector* vector, void* item);
void* vector_at(Vector* vector, size_t index);
void vector_delete(Vector* vector);


#endif  // _MINCC_VECTOR_H_
