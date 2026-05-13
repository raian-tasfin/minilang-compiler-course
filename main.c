#include <stdio.h>
#include <stdlib.h>
#include "cli/cli.h"
#include "lexer/lexer_util.h"
#include "parser/parser.tab.h"
#include "lexer/lex.yy.h"
#include "ast/ast.h"

int main(int argc, char * const * argv)
{
    int exit_status = EXIT_SUCCESS;

    /*********************
     * CLI Options Setup *
     *********************/
    struct cli_opts cliopts = cli_get_opts(argc, argv);
    if (cliopts.err) {
        exit_status = EXIT_FAILURE;
        goto destruct;
    }


    /************************
     * Lexer Report Context *
     ************************/
    struct lxr_ctx lxr_ctx = lxr_ctx_init(&cliopts);
    if (lxr_ctx.err) {
        exit_status = EXIT_FAILURE;
        goto destruct;
    }


    /****************
     * Init Scanner *
     ****************/
    yyscan_t scanner;
    if (yylex_init(&scanner) != 0) {
        fprintf(stderr, "Failed to init lexer\n");
        exit_status = EXIT_FAILURE;
        goto destruct;
    }


    /***************************
     * Attach Context to Lexer *
     ***************************/
    yyset_extra(&lxr_ctx, scanner);
    yyset_in(lxr_ctx.instream, scanner);


    /************
     * AST Root *
     ************/
    struct ast_node * ast_root = NULL;


    /***************
     * Main Matter *
     ***************/
    yyparse(scanner, &ast_root);
    /* fprintf(stdout, */
    /*         "-------------------------\n" */
    /*         "-       AST             -\n" */
    /*         "-------------------------\n"); */
    /* ast_print_texttree(ast_root, stdout); */

    /* FILE * fdot = fopen("report/ast.dot", "w"); */
    /* ast_print_dot(ast_root, fdot); */
    /* fclose(fdot); */


    /************
     * Destruct *
     ************/
    /* ast
     * scanner
     * lexer report context
     * cli options
     */
 destruct:
    ast_delete(&ast_root);
    yylex_destroy(scanner);
    if (lxr_ctx.rprt && lxr_ctx.rprt != stdout) fclose(lxr_ctx.rprt);
    return exit_status;
}
