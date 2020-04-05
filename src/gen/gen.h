#ifndef _GEN_H_
#define _GEN_H_


#include <stdio.h>
#include "../parser/ast.h"


void print_code(FILE* file_ptr, AstList* astlist);


#endif  // _GEN_H_
