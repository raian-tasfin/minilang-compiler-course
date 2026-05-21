%top{
#include "../parser/parser.tab.h"
struct lxr_ctx;
}

%{
#include <stdio.h>
#include "lexer_util.h"
%}

%option noyywrap
%option reentrant
%option extra-type="struct lxr_ctx *"
%option bison-bridge
%option bison-locations

cont        \\[[:blank:]]*\n

%%

[[:blank:]]+             { lxr_process(LEX_BLNK, yylval, yylloc, yyscanner); }
{cont}                   { lxr_process(LEX_CONT, yylval, yylloc, yyscanner); }

"print"                  { lxr_process(PRNT, yylval, yylloc, yyscanner); }

[[:digit:]]+             { lxr_process(INTEGER, yylval, yylloc, yyscanner); }

"+"                      { lxr_process(ADD, yylval, yylloc, yyscanner); }
"-"                      { lxr_process(SUB, yylval, yylloc, yyscanner); }
"*"                      { lxr_process(MUL, yylval, yylloc, yyscanner); }
"/"                      { lxr_process(DIV, yylval, yylloc, yyscanner); }
"%"                      { lxr_process(MOD, yylval, yylloc, yyscanner); }

"("                      { lxr_process(LPRN, yylval, yylloc, yyscanner); }
")"                      { lxr_process(RPRN, yylval, yylloc, yyscanner); }

\n                       { lxr_process(NEWLINE, yylval, yylloc, yyscanner); }

.                        { lxr_process(LEX_ERR, yylval, yylloc, yyscanner); }

%%
