%top{
#include "../parser/parser.tab.h"
}

%{
#include <stdio.h>
%}

%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations

expr_tail   [[:blank:]]*\n?
cont        \\[[:blank:]]*\n

%%

[[:blank:]]+             { printf("LEX_BLNK\n"); }
{cont}                   { printf("LEX_CONT\n"); }

[[:digit:]]+             { printf("INTEGER\n"); return ANY; }

"+"{expr_tail}           { printf("ADD\n"); return ANY; }
"-"{expr_tail}           { printf("SUB\n"); return ANY; }
"*"{expr_tail}           { printf("MUL\n"); return ANY; }
"/"{expr_tail}           { printf("DIV\n"); return ANY; }
"%"{expr_tail}           { printf("MOD\n"); return ANY; }

"("{expr_tail}           { printf("LPRN\n"); return ANY; }
{expr_tail}")"           { printf("RPRN\n"); return ANY; }

\n                       { printf("NEWLINE\n"); }

.                        { printf("LEX_ERR\n"); return ANY; }

%%
