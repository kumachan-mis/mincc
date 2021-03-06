#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/common/vector.h"
#include "../src/common/memory.h"


int main() {
    Vector* vector = vector_new();
    vector_reserve(vector, 4);
    assert(vector->size == 0);
    assert(vector->capacity == 4);

    int* ptr = NULL;

    vector_push_back(vector, int_new(5));
    vector_push_back(vector, int_new(0));
    ptr = (int*)vector_at(vector, 0);
    assert(*ptr == 5);

    vector_push_back(vector, int_new(-1));
    assert(vector->size == 3);
    assert(vector->capacity == 4);
    ptr = (int*)vector_at(vector, 2);
    assert(*ptr == -1);
    ptr = (int*)vector_at(vector, 0);
    assert(*ptr == 5);

    vector_push_back(vector, int_new(8));
    vector_push_back(vector, int_new(1));
    vector_push_back(vector, int_new(4));
    assert(vector->size == 6);
    assert(vector->capacity == 8);
    ptr = (int*)vector_at(vector, 4);
    assert(*ptr == 1);
    ptr = (int*)vector_at(vector, 5);
    assert(*ptr == 4);
    ptr = (int*)vector_at(vector, 6);
    assert(ptr == NULL);
    ptr = (int*)vector_at(vector, 7);
    assert(ptr == NULL);

    free(vector_at(vector, 3));
    vector_erase(vector, 3);
    ptr = (int*)vector_at(vector, 3);
    assert(*ptr == 1);
    assert(vector->size == 5);
    free(vector_at(vector, 3));
    vector_erase(vector, 3);
    ptr = (int*)vector_at(vector, 3);
    assert(*ptr == 4);
    assert(vector->size == 4);

    free(vector_at(vector, 1));
    vector_assign_at(vector, 1, int_new(9));
    ptr = (int*)vector_at(vector, 0);
    assert(*ptr == 5);
    ptr = (int*)vector_at(vector, 1);
    assert(*ptr == 9);
    ptr = (int*)vector_at(vector, 2);
    assert(*ptr == -1);

    vector_delete(vector);

    fprintf(stdout, "OK\n");
    return 0;
}
