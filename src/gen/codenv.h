#ifndef _CODENV_H_
#define _CODENV_H_


#include "../common/vector.h"


typedef struct {
    char* funcname;
    int num_labels;
    char* continue_label;
    char* break_label;
    Vector* codes;
} CodeEnv;


// code-environment
CodeEnv* codenv_new(char* funcname);
char* codenv_create_label(CodeEnv* env);
void codenv_delete(CodeEnv* env);
void append_code(Vector* codes, char* format, ...);


#endif  // _CODENV_H_
