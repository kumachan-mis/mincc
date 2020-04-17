#ifndef _CAST_H_
#define _CAST_H_


#include "../parser/ast.h"


void apply_inplace_integer_promotion(Ast* ast);
void apply_inplace_usual_arithmetic_conversion(Ast* ast);
void apply_inplace_array_to_ptr_conversion(Ast* ast);
void apply_inplace_function_declaration_conversion(Ast* ast);

#endif  // _CAST_H_
