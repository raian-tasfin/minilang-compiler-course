#include <stdio.h>
#include <stdlib.h>
#include "cli/cli.h"
#include "lexer/lexer_util.h"
#include "parser/parser.tab.h"
#include "lexer/lex.yy.h"
#include "ast/ast.h"
#include "symtable/symtable.h"
#include "intrep/interp.h"
#include "cg/cg.h"
#include "darr/darr.h"


int main(int argc, char * const * argv)
{
    struct cli_opts cliopts = {0};
    struct lxr_ctx lxr_ctx = {0};
    /* struct ir_ctx ir_ctx = {0}; */
    yyscan_t scanner = NULL;
    struct ast_node * ast_root = NULL;
    struct ast_ctx ast_ctx = {0};
    /* struct cg_ctx *  cg_ctx = NULL; */
    /* struct darr * program = NULL; */
    int exit_status = EXIT_SUCCESS;
    /* FILE * cg_out = NULL; */

    /*********************
     * CLI Options Setup *
     *********************/
    cliopts = cli_get_opts(argc, argv);
    if (cliopts.err) {
        exit_status = EXIT_FAILURE;
        goto destruct;
    }


    /************************
     * Lexer Report Context *
     ************************/
    lxr_ctx = lxr_ctx_init(&cliopts);
    if (lxr_ctx.err) {
        exit_status = EXIT_FAILURE;
        goto destruct;
    }

    /****************
     * Init Scanner *
     ****************/
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


    /***************
     * Main Matter *
     ***************/
    if (yyparse(scanner, &ast_root) != 0) {
        exit_status = EXIT_FAILURE;
        goto destruct;
    }


   /*****************
    * AST Reporting *
    *****************/
    ast_ctx = ast_ctx_init(cliopts.ast);
    if (ast_ctx.dot) ast_print_dot(ast_root, ast_ctx.dot);
    if (ast_ctx.text) ast_print_texttree(ast_root, ast_ctx.text);



   /* /\******************************* */
   /*  * Intermediate Representation * */
   /*  *******************************\/ */
   /*  struct ir_block * ir_block = ir_block_generate(ast_root); */

   /*  /\**************** */
   /*   * IR Reporting * */
   /*   ****************\/ */
   /*  ir_ctx = ir_ctx_init(&cliopts); */
   /*  if (ir_ctx.err) { */
   /*      exit_status = EXIT_FAILURE; */
   /*      goto destruct; */
   /*  } */
   /*  ir_print(&ir_ctx, ir_block); */

   /*  /\******************* */
   /*   * Code Generation * */
   /*   *******************\/ */
   /*  if (cliopts.cg.generate) { */
   /*      /\* Ensure options *\/ */
   /*      if (!cliopts.cg.outpath) { */
   /*          exit_status = EXIT_FAILURE; */
   /*          goto destruct; */
   /*      } */
   /*      /\* Ensure context *\/ */
   /*      if (!(cg_ctx = cg_ctx_init(ir_block))) { */
   /*          exit_status = EXIT_FAILURE; */
   /*          goto destruct; */
   /*      } */
   /*      program = cg_generate_code(cg_ctx); */
   /*      /\* Ensure output file *\/ */
   /*      if (!(cg_out = fopen(cliopts.cg.outpath, "wb"))) { */
   /*          exit_status = EXIT_FAILURE; */
   /*          goto destruct; */
   /*      } */
   /*      fwrite(cg_buffer(program), */
   /*             darr_elem_size(program), */
   /*             darr_size(program), cg_out); */
   /*  } */

    /************
     * Destruct *
     ************/
    /* ir report context
     * ast
     * scanner
     * lexer report context
     * cli options
     */
 destruct:
    if (ast_root) ast_delete(&ast_root);
    if (scanner) yylex_destroy(scanner);
    if (lxr_ctx.rprt && lxr_ctx.rprt != stdout) fclose(lxr_ctx.rprt);
    if (ast_ctx.dot) fclose(ast_ctx.dot);
    if (ast_ctx.text) fclose(ast_ctx.text);
    /* if (ir_ctx.rprt && ir_ctx.rprt != stdout) fclose(ir_ctx.rprt); */
    /* cg_ctx_destroy(&cg_ctx); */
    /* darr_destroy(&program); */
    /* if (cg_out) fclose(cg_out); */
    return exit_status;
}
