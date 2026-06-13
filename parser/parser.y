%code requires {
#include <stdio.h>
#include <stdbool.h>
#include "../ast/ast.h"
#include "../ast/ast_kind.h"
#include "../darr/darr.h"
#include "../types/scalars.h"
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

static inline struct ast_src_loc
ast_loc_span(YYLTYPE a, YYLTYPE b)
{
    return (struct ast_src_loc){
        .first_line   = a.first_line,
        .first_column = a.first_column,
        .last_line    = b.last_line,
        .last_column  = b.last_column,
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

%token NOT

%token LT
%token LE
%token GT
%token GE
%token NE
%token EQ

%token LPRN
%token RPRN
%token PRNT
%token LBRACE
%token RBRACE

%token TYPE_SPEC_INTEGER
%token TYPE_SPEC_BOOLEAN
%token ASSIGN
%token <char*> IDENT


/*******************************
 * Precedence & Associativity  *
 *******************************/
/* Lowest precedence */
%left OR
%left XOR
%left AND

/* Comparison */
%left EQ NE
%left LT LE GT GE

/* Arithmetic */
%left ADD SUB
%left MUL DIV MOD

/* Unary - highest precedence, right-associative */
%right UMINUS NOT


/*******************************
 * Nonterminal Semantic Types  *
 *******************************/
%type <struct ast_node *> expr program stmt block block_body decl assign


/***********
 * Grammar *
 ***********/
%%

program:
            %empty           { $$ = ast_ctr_block(NULL); *ast_root = $$; }
| program NEWLINE            { $$ = $1; }
| program stmt               { darr_push_back(($1)->block.statements, &$2); $$ = $1; *ast_root = $$; }
;

stmt:
  expr NEWLINE               { $$ = $1; }
| PRNT LPRN expr RPRN        { $$ = ast_ctr_prnt($3, NULL, ast_loc_from_bison(@1)); }
| block                      { $$ = $1; }
| decl
| assign
;

block:
  LBRACE RBRACE                       { $$ = ast_ctr_block(NULL); }
| LBRACE NEWLINE RBRACE               { $$ = ast_ctr_block(NULL); }
| LBRACE block_body RBRACE            { $$ = $2; }
| LBRACE NEWLINE block_body RBRACE    { $$ = $3; }
;

block_body:
  stmt                       { $$ = ast_ctr_block(NULL); darr_push_back($$->block.statements, &$1); }
| block_body stmt            { darr_push_back(($1)->block.statements, &$2); $$ = $1; }
| block_body NEWLINE         { $$ = $1; }
;

decl:
  TYPE_SPEC_INTEGER IDENT NEWLINE                { $$ = ast_ctr_decl(SCAL_INTEGER,  $2, NULL, NULL, ast_loc_span(@1,@2)); }
| TYPE_SPEC_BOOLEAN IDENT NEWLINE                { $$ = ast_ctr_decl(SCAL_BOOLEAN, $2, NULL, NULL, ast_loc_span(@1,@2)); }
| TYPE_SPEC_INTEGER IDENT ASSIGN expr NEWLINE    { $$ = ast_ctr_decl(SCAL_INTEGER,  $2, $4,   NULL, ast_loc_span(@1,@4)); }
| TYPE_SPEC_BOOLEAN IDENT ASSIGN expr NEWLINE    { $$ = ast_ctr_decl(SCAL_BOOLEAN, $2, $4,   NULL, ast_loc_span(@1,@4)); }
;

assign:
  IDENT ASSIGN expr NEWLINE  { $$ = ast_ctr_asn($1, $3, NULL, ast_loc_span(@1,@3)); }
;

expr:
  INTEGER               { $$ = ast_ctr_integer($1, NULL, ast_loc_from_bison(@1)); }
| BOOLEAN               { $$ = ast_ctr_boolean($1, NULL, ast_loc_from_bison(@1)); }
| IDENT                 { $$ = ast_ctr_ident($1, NULL, ast_loc_from_bison(@1));   }
| expr ADD  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(ADD), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr SUB  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(SUB), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr MUL  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(MUL), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr DIV  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(DIV), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr MOD  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(MOD), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr AND  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(AND), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr OR   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(OR),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr XOR  expr        { $$ = ast_ctr_binop(astk_binop_from_tok(XOR), $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr EQ   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(EQ),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr NE   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(NE),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr LT   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(LT),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr LE   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(LE),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr GT   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(GT),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| expr GE   expr        { $$ = ast_ctr_binop(astk_binop_from_tok(GE),  $1, $3, NULL, ast_loc_span(@1, @3)); }
| SUB expr %prec UMINUS { $$ = ast_ctr_unop(astk_unop_from_tok(SUB),  $2, NULL, ast_loc_span(@1, @2)); }
| NOT expr              { $$ = ast_ctr_unop(astk_unop_from_tok(NOT),  $2, NULL, ast_loc_span(@1, @2)); }
| LPRN expr RPRN        { $$ = $2; }
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
