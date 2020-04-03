#ifndef _CODENV_H_
#define _CODENV_H_


#include "../common/vector.h"


typedef struct {
    char* funcname;
    int num_labels;
    Vector* codes;
} CodeEnvironment;


// code-environment
CodeEnvironment* code_environment_new();
void append_code(Vector* codes, char* format, ...);
char* create_new_label(CodeEnvironment* env);
void code_environment_delete(CodeEnvironment* env);


#endif  // _CODENV_H_
