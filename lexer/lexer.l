%top{
#include "../parser/parser.tab.h"
}

%{
#include <stdio.h>
#include "lexer_util.h"
%}

%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations

expr_tail   [[:blank:]]*\n?
cont        \\[[:blank:]]*\n

%%

[[:blank:]]+             { lxr_process(LEX_BLNK, yylval, yylloc, yyscanner); }
{cont}                   { lxr_process(LEX_CONT, yylval, yylloc, yyscanner); }

[[:digit:]]+             { lxr_process(INTEGER, yylval, yylloc, yyscanner); }

"+"{expr_tail}           { lxr_process(ADD, yylval, yylloc, yyscanner); }
"-"{expr_tail}           { lxr_process(SUB, yylval, yylloc, yyscanner); }
"*"{expr_tail}           { lxr_process(MUL, yylval, yylloc, yyscanner); }
"/"{expr_tail}           { lxr_process(DIV, yylval, yylloc, yyscanner); }
"%"{expr_tail}           { lxr_process(MOD, yylval, yylloc, yyscanner); }

"("{expr_tail}           { lxr_process(LPRN, yylval, yylloc, yyscanner); }
{expr_tail}")"           { lxr_process(RPRN, yylval, yylloc, yyscanner); }

\n                       { lxr_process(NEWLINE, yylval, yylloc, yyscanner); }

.                        { lxr_process(LEX_ERR, yylval, yylloc, yyscanner); }

%%
