%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
%}
%option noyywrap
%x FSTRING FSTRING_EXPR
%%
"int"          { printf("token: %15s\t%25s\n", "INTEGER_SPECIFIER", yytext); return INTEGER_SPECIFIER; }
"bool"         { printf("token: %15s\t%25s\n", "BOOLEAN_SPECIFIER", yytext); return BOOLEAN_SPECIFIER; }
"while"        { printf("token: %15s\t%25s\n", "WHILE",             yytext); return WHILE;             }
"if"           { printf("token: %15s\t%25s\n", "IF",                yytext); return IF;                }
"elseif"       { printf("token: %15s\t%25s\n", "ELSEIF",            yytext); return ELSEIF;            }
"else"         { printf("token: %15s\t%25s\n", "ELSE",              yytext); return ELSE;              }
"print"        { printf("token: %15s\t%25s\n", "PRINT",             yytext); return PRINT;             }
"true"|"false" { printf("token: %15s\t%25s\n", "BOOLEAN",           yytext); return BOOLEAN;           }
"nil"          { printf("token: %15s\t%25s\n", "NIL",               yytext); return NIL;               }
"{"            { printf("token: %15s\t%25s\n", "LBLOCK",            yytext); return LBLOCK;            }
"}"            { printf("token: %15s\t%25s\n", "RBLOCK",            yytext); return RBLOCK;            }
[a-zA-Z_][a-zA-Z0-9_]* { printf("token: %15s\t%25s\n", "IDENTIFIER",   yytext); return IDENTIFIER;   }
":="           { printf("token: %15s\t%25s\n", "ASSIGNMENT_OP",     yytext); return ASSIGNMENT_OP;     }
"("            { printf("token: %15s\t%25s\n", "LPAREN",            yytext); return LPAREN;            }
")"            { printf("token: %15s\t%25s\n", "RPAREN",            yytext); return RPAREN;            }
"=="           { printf("token: %15s\t%25s\n", "EQ",                yytext); return EQ;                }
"<="           { printf("token: %15s\t%25s\n", "LEQ",               yytext); return LEQ;               }
">="           { printf("token: %15s\t%25s\n", "GEQ",               yytext); return GEQ;               }
"!="           { printf("token: %15s\t%25s\n", "NEQ",               yytext); return NEQ;               }
"+"            { printf("token: %15s\t%25s\n", "ADD",               yytext); return ADD;               }
"-"            { printf("token: %15s\t%25s\n", "SUB",               yytext); return SUB;               }
"*"            { printf("token: %15s\t%25s\n", "MUL",               yytext); return MUL;               }
"/"            { printf("token: %15s\t%25s\n", "DIV",               yytext); return DIV;               }
"<"            { printf("token: %15s\t%25s\n", "LE",                yytext); return LE;                }
">"            { printf("token: %15s\t%25s\n", "GE",                yytext); return GE;                }
"!"            { printf("token: %15s\t%25s\n", "NOT",               yytext); return NOT;               }
[0-9]+         { printf("token: %15s\t%25s\n", "NUMBER",            yytext); return NUMBER;            }
"/*"([^*]|\*+[^*/])*\*+"/" { /* block comment, skip */ }
\n             { printf("token: %15s\t%25s\n", "NEWLINE",           "\\n"); return NEWLINE;            }
[ \t\r]+       { /* whitespace */ }

    /* ── fstring ── */
\"             { printf("token: %15s\t%25s\n", "STRING_DELIM", "\""); BEGIN(FSTRING); return STRING_DELIM; }

<FSTRING>{
\"             { printf("token: %15s\t%25s\n", "STRING_DELIM", "\""); BEGIN(INITIAL); return STRING_DELIM; }
"{"            { printf("token: %15s\t%25s\n", "LBRACE", "{"); BEGIN(FSTRING_EXPR); return LBRACE; }
\\[ntr\"\\]    { printf("token: %15s\t%25s\n", "ESCAPE", yytext); return ESCAPE; }
[^\"\\{}\n]+   { printf("token: %15s\t%25s\n", "TEXT",   yytext); return TEXT;   }
\n             { printf("Lexical Error: unterminated string\n"); BEGIN(INITIAL); }
}

    /* ── inside {expr} interpolation — lex like normal code ── */
<FSTRING_EXPR>{
"}"            { printf("token: %15s\t%25s\n", "RBRACE", "}"); BEGIN(FSTRING); return RBRACE; }
[a-zA-Z_][a-zA-Z0-9_]* { printf("token: %15s\t%25s\n", "IDENTIFIER", yytext); return IDENTIFIER; }
[0-9]+         { printf("token: %15s\t%25s\n", "NUMBER",     yytext); return NUMBER;     }
"true"|"false" { printf("token: %15s\t%25s\n", "BOOLEAN",    yytext); return BOOLEAN;    }
"+"            { printf("token: %15s\t%25s\n", "ADD",        yytext); return ADD;        }
"-"            { printf("token: %15s\t%25s\n", "SUB",        yytext); return SUB;        }
"*"            { printf("token: %15s\t%25s\n", "MUL",        yytext); return MUL;        }
"/"            { printf("token: %15s\t%25s\n", "DIV",        yytext); return DIV;        }
"<="           { printf("token: %15s\t%25s\n", "LEQ",        yytext); return LEQ;        }
">="           { printf("token: %15s\t%25s\n", "GEQ",        yytext); return GEQ;        }
"=="           { printf("token: %15s\t%25s\n", "EQ",         yytext); return EQ;         }
"!="           { printf("token: %15s\t%25s\n", "NEQ",        yytext); return NEQ;        }
"<"            { printf("token: %15s\t%25s\n", "LE",         yytext); return LE;         }
">"            { printf("token: %15s\t%25s\n", "GE",         yytext); return GE;         }
"!"            { printf("token: %15s\t%25s\n", "NOT",        yytext); return NOT;        }
"("            { printf("token: %15s\t%25s\n", "LPAREN",     yytext); return LPAREN;     }
")"            { printf("token: %15s\t%25s\n", "RPAREN",     yytext); return RPAREN;     }
[ \t]+         { /* whitespace inside interpolation */ }
\n             { printf("Lexical Error: newline inside interpolation\n"); BEGIN(INITIAL); }
.              { printf("Lexical Error: unexpected character '%s' in interpolation\n", yytext); }
}

.              { printf("Lexical Error: unexpected character '%s'\n", yytext); }
%%
