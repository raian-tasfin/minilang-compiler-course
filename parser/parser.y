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

%token ANY

%%

input:
    | input ANY   { /* ignore everything */ }
    ;

%%

void yyerror(YYLTYPE *loc, void *scanner, const char *s)
{
    fprintf(stderr, "parse error: %s\n", s);
}
