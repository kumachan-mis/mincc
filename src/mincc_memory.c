#include "mincc_memory.h"

#include <stdio.h>
#include <stdlib.h>


void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: failed to allocate memory\n");
        exit(1);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t new_size) {
    ptr = realloc(ptr, new_size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: failed to allocate memory\n");
        exit(1);
    }
    return ptr;
}
