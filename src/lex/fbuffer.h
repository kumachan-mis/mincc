#ifndef _FREADER_H_
#define _FREADER_H_


#include <stdio.h>


typedef struct {
    char* data;
    int len;
    int pos;
} FileBuffer;

// fbuffer
FileBuffer* fbuffer_new(FILE* file_ptr);
int fbuffer_starts_with(FileBuffer* fbuffer, char* str);
char fbuffer_top(FileBuffer* fbuffer);
void fbuffer_pop(FileBuffer* fbuffer);
void fbuffer_popn(FileBuffer* fbuffer, int n);
void fbuffer_delete(FileBuffer* fbuffer);


#endif  // _FREADER_H_
