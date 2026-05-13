#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.tab.h"
#include "lexer/lex.yy.h"

int main()
{
    /****************
     * Init Scanner *
     ****************/
    yyscan_t scanner;
    if (yylex_init(&scanner) != 0) {
        fprintf(stderr, "Failed to init lexer\n");
        return EXIT_FAILURE;
    }
    yyset_in(stdin, scanner);



    /***************
     * Main Matter *
     ***************/
    yyparse(scanner);



    /********************
     * Destruct Scanner *
     ********************/
    return EXIT_SUCCESS;
}
