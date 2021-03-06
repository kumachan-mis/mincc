#ifndef _MEMORY_H_
#define _MEMORY_H_


#include <stddef.h>


void* safe_malloc(size_t size);
void* safe_realloc(void* ptr, size_t new_size);
int* int_new(int n);
char* str_new(char* str);


#endif  // _MEMORY_H_
