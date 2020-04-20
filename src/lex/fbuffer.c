#include "fbuffer.h"

#include <stdlib.h>
#include <string.h>
#include "../common/memory.h"


// fbuffer
FileBuffer* fbuffer_new(FILE* file_ptr) {
    fseek(file_ptr, 0, SEEK_END);
    int size = ftell(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);

    char* data = safe_malloc((size + 1) * sizeof(char));
    fread(data, sizeof(char), size, file_ptr);
    data[size] = '\0';

    FileBuffer* fbuffer = safe_malloc(sizeof(FileBuffer));
    fbuffer->data = data;
    fbuffer->len = size;
    fbuffer->pos = 0;
    return fbuffer;
}

int fbuffer_starts_with(FileBuffer* fbuffer, char* str) {
    if (fbuffer->pos >= fbuffer->len) return 0;

    char* data = fbuffer->data;
    int pos =  fbuffer->pos;
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (data[pos + i] == '\0' || data[pos + i] != str[i]) {
            return 0;
        }
    }
    return 1;
}

char fbuffer_top(FileBuffer* fbuffer) {
    if (fbuffer->pos >= fbuffer->len) return '\0';
    return fbuffer->data[fbuffer->pos];
}

void fbuffer_pop(FileBuffer* fbuffer) {
    fbuffer->pos++;
}

void fbuffer_popn(FileBuffer* fbuffer, int n) {
    fbuffer->pos += n;
}

void fbuffer_delete(FileBuffer* fbuffer) {
    if (fbuffer == NULL) return;

    free(fbuffer->data);
    free(fbuffer);
}
