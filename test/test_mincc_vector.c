#include <assert.h>
#include <stdio.h>
#include "../src/mincc_vector.h"
#include "../src/mincc_memory.h"


int* safe_malloc_int(int value) {
    int* ptr = (int*)safe_malloc(sizeof(int));
    *ptr = value;
    return ptr;
}

int main() {
    Vector* vector = vector_new();
    vector_reserve(vector, 4);
    assert(vector->size == 0);
    assert(vector->capacity == 4);

    int* ptr = NULL;

    vector_push_back(vector, safe_malloc_int(5));
    vector_push_back(vector, safe_malloc_int(0));
    ptr = (int*)vector_at(vector, 0);
    assert(*ptr == 5);

    vector_push_back(vector, safe_malloc_int(-1));
    assert(vector->size == 3);
    assert(vector->capacity == 4);
    ptr = (int*)vector_at(vector, 2);
    assert(*ptr == -1);
    ptr = (int*)vector_at(vector, 0);
    assert(*ptr == 5);

    vector_push_back(vector, safe_malloc_int(8));
    vector_push_back(vector, safe_malloc_int(1));
    vector_push_back(vector, safe_malloc_int(4));
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

    vector_delete(vector);

    fprintf(stdout, "OK\n");
    return 0;
}
