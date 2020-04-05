#include <stdio.h>
#include <stdlib.h>
#include "lex/lex.h"
#include "parser/parser.h"
#include "gen/gen.h"


FILE* safe_fopen(char* filename, char* mode);


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Error: ivalid command-line arguments\n");
        return 1;
    }

    FILE* input_file_ptr = safe_fopen(argv[1], "r");
    TokenList* tokenlist = tokenize(input_file_ptr);
    fclose(input_file_ptr);

    AstList* astlist = parse(tokenlist);

    FILE* output_file_ptr = safe_fopen(argv[2], "w");
    print_code(output_file_ptr, astlist);
    fclose(output_file_ptr);

    astlist_delete(astlist);
    tokenlist_delete(tokenlist);
    return 0;
}

FILE* safe_fopen(char* filename, char* mode) {
    FILE* file_ptr = fopen(filename, mode);
    if (file_ptr == NULL) {
        fprintf(stderr, "Error: failed to open file %s\n", filename);
        exit(1);
    }
    return file_ptr;
}
