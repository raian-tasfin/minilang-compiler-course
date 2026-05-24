#include <stdio.h>

#ifndef AST_KIND_H
#define AST_KIND_H 1


/* Types of nodes with values */
enum ast_kind {
    AST_INTEGER,
    AST_BINOP,
    AST_PRNT,
    AST_BLOCK,
    AST_PUNCTUATOR,
};


/* Types of nodes that have no values */
enum ast_punctuator_type {
    AST_ERR,
};

/* Binary operators */
enum ast_binop_type {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
};


char *
astk_kind_to_str(enum ast_kind kind);

char *
astk_punc_to_str(enum ast_punctuator_type type);

char *
astk_binop_to_str(enum ast_binop_type type);

enum ast_binop_type
astk_binop_from_tok(int token_type);


#endif
// AST_KIND_H
