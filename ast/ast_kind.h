#include <stdio.h>

#ifndef AST_KIND_H
#define AST_KIND_H 1


enum ast_kind {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_ERR,
    AST_PRNT,
    AST_INTEGER,
};


enum ast_kind
astk_from_tok(int token_type);

char *
astk_tokstr(enum ast_kind type);


#endif
// AST_KIND_H
