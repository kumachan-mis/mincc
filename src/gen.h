#ifndef _GEN_H_
#define _GEN_H_


#include "parser.h"
#include "vector.h"
#include "map.h"


typedef struct {
    char* funcname;
    int num_labels;
    int stack_offset;
    Map* var_map;
    Vector* codes;
} CodeEnvironment;

void print_code(AstList* astlist);


#endif  // _GEN_H_
