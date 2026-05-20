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


/*******************************
 * Precedence & Associativity  *
 *******************************/
%left ADD SUB
%left MUL DIV MOD


/*******************************
 * Nonterminal Semantic Types  *
 *******************************/
%type <struct ast_node *> expr line program stmt

%%

program:
%empty           { $$ = NULL; }
| program line   { $$ = $2;   }
;

line:
NEWLINE          { $$ = NULL; }
| stmt NEWLINE   { $$ = $1; *ast_root = $1; }
| stmt           { $$ = $1;   }
;

stmt:
  expr                 { $$ = $1; }
| PRNT LPRN expr RPRN  { $$ = ast_ctr_prnt($3); }
;


expr:
INTEGER           { $$ = ast_ctr_integer($1); }
| expr ADD expr   { $$ = ast_ctr_binop(astk_from_tok(ADD), $1, $3);  }
| expr SUB expr   { $$ = ast_ctr_binop(astk_from_tok(SUB), $1, $3);  }
| expr MUL expr   { $$ = ast_ctr_binop(astk_from_tok(MUL), $1, $3);  }
| expr DIV expr   { $$ = ast_ctr_binop(astk_from_tok(DIV), $1, $3);  }
| expr MOD expr   { $$ = ast_ctr_binop(astk_from_tok(MOD), $1, $3);  }
| LPRN expr RPRN  { $$ = $2; }
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
