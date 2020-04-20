#include "vector.h"

#include <stdlib.h>
#include "memory.h"


Vector* vector_extend(Vector* vector);


Vector* vector_new() {
    Vector* vector = safe_malloc(sizeof(Vector));
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    return vector;
}

void vector_reserve(Vector* vector, size_t size) {
    if (vector->size == size) return;
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
            vector = vector_extend(vector);
        }
        vector->data[vector->size] = item;
        vector->size++;
    }
}

void vector_join(Vector* dist, Vector* src) {
    vector_reserve(dist, dist->size + src->size);
    size_t i = 0, size = src->size;
    for (i = 0; i < size; i++) {
        vector_push_back(dist, vector_at(src, i));
    }
    free(src->data);
    src->data = NULL;
    src->size = 0;
    src->capacity = 0;
}

vector_item_t vector_at(Vector* vector, size_t index) {
    if (index < 0 || index >= vector->size) return NULL;
    return vector->data[index];
}

void vector_delete(Vector* vector) {
    if (vector == NULL) return;

    size_t size = vector->size;
    size_t i;
    for (i = 0; i < size; ++i) {
        free(vector->data[i]);
    }
    free(vector->data);
    free(vector);
}

Vector* vector_extend(Vector* vector) {
    size_t new_capacity = 2*vector->capacity;
    vector->data = (vector_item_t*)safe_realloc(
        vector->data, new_capacity * sizeof(vector_item_t)
    );
    vector->capacity = new_capacity;
    return vector;
}
