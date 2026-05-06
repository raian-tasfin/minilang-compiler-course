%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

int line_num = 1;
%}

%option noyywrap

%%
"var"                   { return VAR; }
"print"                 { return PRINT; }

[a-zA-Z_][a-zA-Z0-9_]*  {
                            yylval.str = strdup(yytext);
                            return IDENTIFIER;
                        }

[0-9]+                  {
                            yylval.num = atoi(yytext);
                            return NUMBER;
                        }

":="                    { return ASSIGN_OP; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MUL; }
"/"                     { return DIV; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }

":>"([^:]|\n|":"[^<])*":<"   {
    int i;
    for (i = 0; yytext[i]; i++)
        if (yytext[i] == '\n') line_num++;
    printf("Skipping block comment ending at line %03d\n", line_num);
}

":>"[^\n]*              { printf("Skipping comment: Line: %03d: %s\n", line_num, yytext); }

\n                      { line_num++; return NEWLINE; }
[ \t\r]+                { /* space, tabls */ }

.                       { printf("Lexical Error on line %d: %s\n", line_num, yytext); }
%%
