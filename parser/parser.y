%code requires {
#include <stdio.h>
}

%code provides {
int yylex(YYSTYPE *yylval, YYLTYPE *yylloc, void *scanner);
void yyerror(YYLTYPE *loc, void *scanner, const char *s);
}

%define api.pure full
%locations
%lex-param { void *scanner }
%parse-param { void *scanner }

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
%token INTEGER
%token ADD
%token SUB
%token MUL
%token DIV
%token MOD
%token LPRN
%token RPRN


/*******************************
 * Precedence & Associativity  *
 *******************************/
%left ADD SUB
%left MUL DIV MOD


%%

program:
%empty
| program line
;

line:
NEWLINE
| expr NEWLINE
;

expr:
    INTEGER
  | expr ADD expr
  | expr SUB expr
  | expr MUL expr
  | expr DIV expr
  | expr MOD expr
  | LPRN expr RPRN
;

%%

/**
 * DO NOT DELETE forward declaration.
 * otherwise we would have to include lex.yy.h making circular
 * dependency
 */
char *yyget_text(void *yyscanner);


void yyerror(YYLTYPE *loc, void *scanner, const char *s)
{
    char *bad_token = yyget_text(scanner);
    fprintf(stderr, "Parse Error at line %d, col %d-%d: %s\n",
            loc->first_line,
            loc->first_column,
            loc->last_column,
            s);
    if (bad_token && bad_token[0] != '\0') {
        fprintf(stderr, " -> Unexpected input: '%s'\n", bad_token);
    }
}
