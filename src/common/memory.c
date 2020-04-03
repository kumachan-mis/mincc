#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

int* int_new(int n) {
    int* ret = (int*)safe_malloc(sizeof(int));
    *ret = n;
    return ret;
}

char* str_new(char* str) {
    char* ret = (char*)safe_malloc((strlen(str) + 1) * sizeof(char));
    strcpy(ret, str);
    return ret;
}
