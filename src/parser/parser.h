#ifndef _PARSER_H_
#define _PARSER_H_


#include "../lex/token.h"
#include "ast.h"


AstList* parse(TokenList* tokenlist);


#endif  // _PARSER_H_
