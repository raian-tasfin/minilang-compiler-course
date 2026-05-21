#ifndef LEXER_UTIL_H
#define LEXER_UTIL_H 1

#include "../parser/parser.tab.h"
#include "../cli/cli.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

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

struct lxr_ctx {
    FILE * rprt;
    FILE * instream;
    int open_parens;
    bool err;
};

struct lxr_ctx
lxr_ctx_init(struct cli_opts * cliopts);


#define LEX_IGNR -1

/* Methods */
char * lxr_toktostr(int token_type);

void
lxr_print_token(int token_type,
                YYSTYPE * yylval,
                struct lxr_ctx * ctx);

void lxr_updt_loc(YYLTYPE * yylloc, void * yyscanner);

int
lxr_process_proc(int token_type,
                 YYSTYPE * yylval,
                 YYLTYPE * yylloc,
                 void * yyscanner,
                 struct lxr_ctx * ctx);

#define lxr_process(t, yylval, yylloc, yyscanner)                   \
    do {                                                            \
        struct lxr_ctx * ctx = yyget_extra(yyscanner);              \
        int ret = lxr_process_proc(t,                               \
                                   yylval,                          \
                                   yylloc,                          \
                                   yyscanner,                       \
                                   ctx);                            \
        if (ret != LEX_IGNR) return ret;                            \
    } while(0)

#endif
