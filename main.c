#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"

extern FILE *yyin;
extern int yylex(void);

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror("fopen");
        return 1;
    }

    if (yyparse() == 0) {
        printf("Parsing successful.\n");
    } else {
        printf("Parsing failed.\n");
    }

    fclose(yyin);
    return 0;
}
