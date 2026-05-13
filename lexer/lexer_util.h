#ifndef LEXER_UTIL_H
#define LEXER_UTIL_H 1

#include "../parser/parser.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LEX_IGNR -1

/* Methods */
#define lxr_toktostr(token_type) #token_type

#define lxr_print_token(token_type, val_ptr)                    \
    fprintf(stdout, "%s", lxr_toktostr(token_type));            \
    if (token_type == INTEGER && val_ptr)                       \
        fprintf(stdout, ": %d", *(val_ptr));                    \
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
