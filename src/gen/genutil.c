#include "genutil.h"

#include <stdlib.h>


void put_code(FILE* file_ptr, Vector* codes) {
    size_t i = 0, size = codes->size;
    for (i = 0; i < size; i++) {
        char* str = (char*)vector_at(codes, i);
        fputs(str, file_ptr);
    }
}

int ilog2(int x) {
    if (x <= 0) {
        fprintf(stderr, "Error: antilog should be opsitive\n");
        exit(1);
    }
    int ret = 0;
    while (x >= 2) {
        ret++;
        x /= 2;
    }
    return ret;
}
