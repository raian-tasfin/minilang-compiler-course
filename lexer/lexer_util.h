#ifndef LEXER_UTIL_H
#define LEXER_UTIL_H 1

#include "../parser/parser.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward-declare yyscan_t so lexer_util.h does not need to pull in
 * lex.yy.h.  lex.yy.h in turn includes parser.tab.h, which would
 * create a circular dependency when ast.h → lexer_util.h is expanded
 * inside the %code requires block that bison emits at the top of
 * parser.tab.h (before YYSTYPE/YYLTYPE are defined).
 * The concrete typedef is in lex.yy.h; this opaque alias is enough
 * for the function prototypes below.  Every .c file that calls these
 * functions must include lex.yy.h before including lexer_util.h. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif

#define LEX_IGNR -1

/* Methods */
char * lxr_toktostr(int token_type);

#define lxr_print_token(token_type, val_ptr)                            \
    fprintf(stdout, "%s", lxr_toktostr(token_type));                    \
    if (token_type == INTEGER && (val_ptr))                             \
        fprintf(stdout, ": %d", (val_ptr)->INTEGER);                    \
    fprintf(stdout, "\n");


void lxr_updt_loc(YYLTYPE *yylloc, void *yyscanner);
int lxr_process_proc(int token_type,
                     YYSTYPE *yylval,
                     YYLTYPE *yylloc,
                     void *yyscanner);

#define lxr_process(t, yylval, yylloc, yyscanner)                   \
    do {                                                            \
        int ret = lxr_process_proc(t, yylval, yylloc, yyscanner);   \
        if (ret != LEX_IGNR) return ret;                            \
    } while(0)

#endif
