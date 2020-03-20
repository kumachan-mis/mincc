#include "mincc_vector.h"

#include <stdlib.h>
#include "mincc_memory.h"


Vector* vector_new() {
    Vector* vector = safe_malloc(sizeof(Vector));
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    return vector;
}

void vector_reserve(Vector* vector, size_t size) {
    vector->data = safe_realloc(vector->data, size * sizeof(void*));
    vector->capacity = size;
    if (vector->size > size) vector->size = size;
}

void vector_push_back(Vector* vector, void* item) {
    if (vector->data == NULL) {
        vector->data = safe_malloc(sizeof(void*));
        vector->data[0] = item;
        vector->size = 1;
        vector->capacity = 1;
    } else {
        if (vector->size == vector->capacity) {
            vector->data = safe_realloc(vector->data, (2*vector->capacity) * sizeof(void*));
            vector->capacity *= 2;
        }
        vector->data[vector->size] = item;
        vector->size++;
    }
}

void* vector_at(Vector* vector, size_t index) {
    if (index < 0 || index >= vector->size) return NULL;
    return vector->data[index];
}

void vector_delete(Vector* vector) {
    free(vector->data);
    free(vector);
}
