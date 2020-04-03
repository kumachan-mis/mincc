#ifndef _VECTOR_H_
#define _VECTOR_H_


#include <stddef.h>


typedef void* vector_item_t;

typedef struct {
    vector_item_t* data;
    size_t size;
    size_t capacity;
} Vector;

typedef void (*DeleteFunc)(void* ptr);

Vector* vector_new();
void vector_reserve(Vector* vector, size_t size);
void vector_push_back(Vector* vector, vector_item_t item);
void vector_join(Vector* dist, Vector* src);
vector_item_t vector_at(Vector* vector, size_t index);
void vector_delete(Vector* vector);


#endif  // _VECTOR_H_
