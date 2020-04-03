#include "codenv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../common/memory.h"


// assertion
void code_limit_error();
void label_limit_error();


// code-environment
CodeEnvironment* code_environment_new() {
    CodeEnvironment* env = (CodeEnvironment*)safe_malloc(sizeof(CodeEnvironment));
    env->funcname = NULL;
    env->num_labels = 0;
    env->codes = vector_new();
    return env;
}

void append_code(Vector* codes, char* format, ...) {
    char buffer[511];
    va_list list;
    va_start(list, format);
    int success = vsnprintf(buffer, 510, format, list);
    va_end(list);
    if (!success) {
        code_limit_error();
        return;
    }
    vector_push_back(codes, str_new(buffer));
}

char* create_new_label(CodeEnvironment* env) {
    if (env->num_labels == 1 << 30) {
        label_limit_error();
        return NULL;
    }
    char* label = safe_malloc(sizeof(char) * (strlen(env->funcname) + 12 + 1));
    sprintf(label, "_%s_%d", env->funcname, env->num_labels);
    env->num_labels++;
    return label;
}

void code_environment_delete(CodeEnvironment* env) {
    free(env->funcname);
    vector_delete(env->codes);
    free(env);
}

// assertion
void code_limit_error() {
    fprintf(stderr, "Error: code is too long\n");
    exit(1);
}

void label_limit_error() {
    fprintf(stderr, "Error: cannot create new label\n");
    exit(1);
}