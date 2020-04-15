#ifndef _CAST_H_
#define _CAST_H_


#include "../parser/ast.h"


void cast_inplace_array_to_ptr(Ast* ast);
void cast_inplace_function_declaration(Ast* ast);

#endif  // _CAST_H_
