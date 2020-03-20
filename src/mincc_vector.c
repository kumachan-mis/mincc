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
    vector->data = (vector_item_t*)safe_realloc(vector->data, size * sizeof(vector_item_t));
    vector->capacity = size;
    if (vector->size > size) vector->size = size;
}

void vector_push_back(Vector* vector, vector_item_t item) {
    if (vector->data == NULL) {
        vector->data = (vector_item_t*)safe_malloc(sizeof(vector_item_t));
        vector->data[0] = item;
        vector->size = 1;
        vector->capacity = 1;
    } else {
        if (vector->size == vector->capacity) {
            vector->data = (vector_item_t*)safe_realloc(
                vector->data, (2*vector->capacity) * sizeof(vector_item_t));
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
    size_t capacity = vector->capacity;
    for (size_t i = 0; i < capacity; ++i) {
        free(vector->data[i]);
    }
    free(vector->data);
    free(vector);
}
