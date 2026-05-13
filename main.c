#include <stdio.h>
#include <stdlib.h>
#include "cli/cli.h"
#include "lexer/lexer_util.h"
#include "parser/parser.tab.h"
#include "lexer/lex.yy.h"
#include "ast/ast.h"

int main(int argc, char * const * argv)
{
    /*********************
     * CLI Options Setup *
     *********************/
    struct cli_opts cliopts = cli_get_opts(argc, argv);
    if (cliopts.err) return EXIT_FAILURE;


    /************************
     * Lexer Report Context *
     ************************/
    struct lxr_rprt_ctx lxrprt_ctx =
        lxr_rprt_ctx_init(cliopts.lxr.rprt, cliopts.lxr.path);
    if (lxrprt_ctx.err) return EXIT_FAILURE;


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


    /************
     * Destruct *
     ************/
    /* ast
     * scanner
     * lexer report context
     * cli options
     */
    ast_delete(&ast_root);
    yylex_destroy(scanner);
    fclose(lxrprt_ctx.strm);

    return EXIT_SUCCESS;
}
