#ifndef _CAST_H_
#define _CAST_H_


#include "../parser/ast.h"


Ast* cast_array_to_ptr(Ast* ast);
void cast_inline_array_to_ptr(Ast* ast);


#endif  // _CAST_H_
