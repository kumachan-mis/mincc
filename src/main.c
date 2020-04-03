#include "lex/lex.h"
#include "parser/parser.h"
#include "gen/gen.h"


int main(int argc, char* argv[]) {
    TokenList* tokenlist = tokenize(stdin);
    AstList* astlist = parse(tokenlist);
    print_code(astlist);
    astlist_delete(astlist);
    tokenlist_delete(tokenlist);
    return 0;
}
