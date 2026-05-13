#include <stdio.h>
#include <stdlib.h>
#include "parser/parser.tab.h"
#include "lexer/lex.yy.h"
#include "ast/ast.h"

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


    /************
     * AST Root *
     ************/
    struct ast_node * ast_root;


    /***************
     * Main Matter *
     ***************/
    yyparse(scanner, &ast_root);
    fprintf(stdout,
            "-------------------------\n"
            "-       AST             -\n"
            "-------------------------\n");
    ast_print_texttree(ast_root, stdout);

    FILE * fdot = fopen("report/ast.dot", "w");
    ast_print_dot(ast_root, fdot);
    fclose(fdot);


    /********************
     * Destruct Scanner *
     ********************/
    return EXIT_SUCCESS;
}
