%code requires {
#include <stdio.h>
#include <stdbool.h>
#include "../ast/ast.h"
#include "../ast/ast_kind.h"
#include "../darr/darr.h"
}


%code provides {
int yylex(YYSTYPE * yylval, YYLTYPE * yylloc, void * scanner);

void
yyerror(YYLTYPE * loc,
        void * scanner,
        struct ast_node ** ast_root,
        const char * s);

 char *yyget_text(void *yyscanner);

static inline struct ast_src_loc
ast_loc_from_bison(YYLTYPE loc)
{
    return (struct ast_src_loc) {
        .first_line   = loc.first_line,
        .first_column = loc.first_column,
        .last_line    = loc.last_line,
        .last_column  = loc.last_column
    };
}

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
%token LEX_BLNK
%token LEX_CONT
%token NEWLINE
%token LEX_ERR


/**********
 * Tokens *
 **********/
%token <int>  INTEGER
%token <bool> BOOLEAN
%token ADD
%token SUB
%token MUL
%token DIV
%token MOD

%token AND
%token OR
%token XOR

%token LPRN
%token RPRN
%token PRNT
%token LBRACE
%token RBRACE


/*******************************
 * Precedence & Associativity  *
 *******************************/

/* Lowest precedence */
%left OR
%left XOR
%left AND

/* Arithmetic */
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
            %empty           { $$ = ast_ctr_block(NULL); *ast_root = $$; }
| program NEWLINE  { $$ = $1; }
| program stmt     { darr_push_back(($1)->block.statements, &$2); $$ = $1; *ast_root = $$; }
;

stmt:
  expr                 { $$ = $1; }
| PRNT LPRN expr RPRN  { $$ = ast_ctr_prnt($3, NULL, ast_loc_from_bison(@1), strdup(yyget_text(scanner))); }
| block                { $$ = $1; }
;


block:
  LBRACE RBRACE                       { $$ = ast_ctr_block(NULL); }
| LBRACE NEWLINE RBRACE               { $$ = ast_ctr_block(NULL); }
| LBRACE block_body RBRACE            { $$ = $2; }
| LBRACE NEWLINE block_body RBRACE    { $$ = $3; }
;

block_body:
  stmt                       { $$ = ast_ctr_block(NULL); darr_push_back($$->block.statements, &$1); }
| block_body NEWLINE stmt    { darr_push_back(($1)->block.statements, &$3); $$ = $1; }
| block_body NEWLINE         { $$ = $1; }
;

expr:
  INTEGER           { $$ = ast_ctr_integer($1, NULL, ast_loc_from_bison(@1), strdup(yyget_text(scanner))); }
| BOOLEAN           { $$ = ast_ctr_boolean($1, NULL, ast_loc_from_bison(@1), strdup(yyget_text(scanner))); }
| expr ADD expr     { $$ = ast_ctr_binop(astk_binop_from_tok(ADD), $1, $3, NULL); }
| expr SUB expr     { $$ = ast_ctr_binop(astk_binop_from_tok(SUB), $1, $3, NULL); }
| expr MUL expr     { $$ = ast_ctr_binop(astk_binop_from_tok(MUL), $1, $3, NULL); }
| expr DIV expr     { $$ = ast_ctr_binop(astk_binop_from_tok(DIV), $1, $3, NULL); }
| expr MOD expr     { $$ = ast_ctr_binop(astk_binop_from_tok(MOD), $1, $3, NULL); }
| expr AND expr     { $$ = ast_ctr_binop(astk_binop_from_tok(AND), $1, $3, NULL); }
| expr OR  expr     { $$ = ast_ctr_binop(astk_binop_from_tok(OR), $1, $3, NULL); }
| expr XOR expr     { $$ = ast_ctr_binop(astk_binop_from_tok(XOR), $1, $3, NULL); }
| LPRN expr RPRN    { $$ = $2; }
;

%%

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
