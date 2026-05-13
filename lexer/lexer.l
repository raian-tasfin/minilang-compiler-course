%{
#include <stdio.h>
%}

%option noyywrap
%option reentrant

expr_tail   [[:blank:]]*\n?
cont        \\[[:blank:]]*\n

%%

[[:blank:]]+             { printf("LEX_BLNK\n"); }
{cont}                   { printf("LEX_CONT\n"); }

[[:digit:]]+             { printf("INTEGER\n"); }

"+"{expr_tail}           { printf("ADD\n"); }
"-"{expr_tail}           { printf("SUB\n"); }
"*"{expr_tail}           { printf("MUL\n"); }
"/"{expr_tail}           { printf("DIV\n"); }
"%"{expr_tail}           { printf("MOD\n"); }

"("{expr_tail}           { printf("LPRN\n"); }
{expr_tail}")"           { printf("RPRN\n"); }

\n                       { printf("NEWLINE\n"); }

.                        { printf("LEX_ERR\n"); }

%%
