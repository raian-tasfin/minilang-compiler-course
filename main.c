#include <stdio.h>
#include <stdlib.h>

extern int yyparse();
extern FILE* yyin;

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ./minicompiler <source_file>\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    yyin = file;

    printf("Starting Parser...\n");

    int returnCode = EXIT_SUCCESS;
    if (yyparse() == 0) {
        printf("Parsing finished successfully!\n");
    } else {
        printf("Parsing failed!\n");
        returnCode = EXIT_FAILURE;
    }

    fclose(file);
    return returnCode;
}
