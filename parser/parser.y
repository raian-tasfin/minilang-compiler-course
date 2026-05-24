%code requires {
#include <stdio.h>
#include "../ast/ast.h"
#include "../ast/ast_kind.h"
}

%code provides {
int yylex(YYSTYPE * yylval, YYLTYPE * yylloc, void * scanner);

void
yyerror(YYLTYPE * loc,
        void * scanner,
        struct ast_node ** ast_root,
        const char * s);
}

%define api.pure full
%define api.value.type union

%locations
%lex-param { void * scanner }

%parse-param { void * scanner }
%parse-param { struct ast_node ** ast_root }


/************************
 * Lexer-Control Tokens *
 ************************/
/* These are not used by the parser, only by the lexer. */
%token LEX_BLNK
%token LEX_CONT
%token NEWLINE
%token LEX_ERR


/**********
 * Tokens *
 **********/
%token <int> INTEGER
%token ADD
%token SUB
%token MUL
%token DIV
%token MOD
%token LPRN
%token RPRN
%token PRNT
%token LBRACE
%token RBRACE


/*******************************
 * Precedence & Associativity  *
 *******************************/
%left ADD SUB
%left MUL DIV MOD


/*******************************
 * Nonterminal Semantic Types  *
 *******************************/
%type <struct ast_node *> expr program stmt block block_body


/***********
 * Grammar *
 ***********/
%%

program:
  %empty           { $$ = NULL; }
| program NEWLINE  { $$ = $1; }
| program stmt     { $$ = ast_ctr_block($1, $2); *ast_root = $$; }
;

stmt:
  expr                 { $$ = $1; }
| PRNT LPRN expr RPRN  { $$ = ast_ctr_prnt($3); }
| block                { $$ = $1; }
;

/*
 * A block is a linked list of block nodes.
 * Each block node: .parent = previous node in list (or NULL if first)
 *                  .child  = the stmt at this position
 *
 * { stmt1 \n stmt2 \n stmt3 }  builds:
 *
 *   block(parent=block(parent=block(parent=NULL, child=stmt1),
 *                      child=stmt2),
 *         child=stmt3)
 *
 * Nested blocks are handled naturally: a stmt can itself be a block,
 * so { { inner } \n stmt } gives a block whose child is another block.
 */
block:
  LBRACE RBRACE
      { $$ = ast_ctr_block(NULL, NULL); }
| LBRACE NEWLINE RBRACE
      { $$ = ast_ctr_block(NULL, NULL); }
| LBRACE block_body RBRACE
      { $$ = $2; }
| LBRACE NEWLINE block_body RBRACE
      { $$ = $3; }
;

block_body:
  stmt
      { $$ = ast_ctr_block(NULL, $1); }
| block_body NEWLINE stmt
      { $$ = ast_ctr_block($1, $3); }
| block_body NEWLINE
      { $$ = $1; }
;

expr:
  INTEGER           { $$ = ast_ctr_integer($1); }
| expr ADD expr     { $$ = ast_ctr_binop(astk_binop_from_tok(ADD), $1, $3); }
| expr SUB expr     { $$ = ast_ctr_binop(astk_binop_from_tok(SUB), $1, $3); }
| expr MUL expr     { $$ = ast_ctr_binop(astk_binop_from_tok(MUL), $1, $3); }
| expr DIV expr     { $$ = ast_ctr_binop(astk_binop_from_tok(DIV), $1, $3); }
| expr MOD expr     { $$ = ast_ctr_binop(astk_binop_from_tok(MOD), $1, $3); }
| LPRN expr RPRN    { $$ = $2; }
;

%%

/**
 * DO NOT DELETE forward declaration.
 * otherwise we would have to include lex.yy.h making circular
 * dependency
 */
char *yyget_text(void *yyscanner);


void
yyerror(YYLTYPE * loc,
        void * scanner,
        struct ast_node ** ast_root,
        const char * s)
{
    char * bad_token = yyget_text(scanner);
    fprintf(stderr, "Parse Error at line %d, col %d-%d: %s\n",
            loc->first_line,
            loc->first_column,
            loc->last_column,
            s);
    if (bad_token && bad_token[0] != '\0') {
        fprintf(stderr, " -> Unexpected input: '%s'\n", bad_token);
    }
}
