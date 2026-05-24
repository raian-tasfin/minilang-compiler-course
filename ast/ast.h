#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ast_kind.h"
#include "../cli/cli.h"

#ifndef AST_H
#define AST_H 1

/***************
 * AST Context *
 ***************/
struct ast_ctx {
    FILE * dot;
    FILE * text;
    bool err;
};

struct ast_ctx ast_ctx_init(struct cli_ast_opts opts);


/*****************
 * AST Structure *
 *****************/
struct ast_binop_node {
    enum ast_binop_type op;
    struct ast_node * left;
    struct ast_node * right;
};

struct ast_block_node {
    struct ast_node * child;
    struct ast_node * parent;
};

struct ast_print_node {
    struct ast_node * child;
};

struct ast_node {
    enum ast_kind type;
    union {
        struct ast_binop_node binop;
        struct ast_block_node block;
        struct ast_print_node print;
        int integer;
        enum ast_punctuator_type punctuator;
    };
};


/*******************
 * Context Methods *
 *******************/
struct ast_ctx ast_ctx_init(struct cli_ast_opts opts);


/************************************
 * AST Constructors and Destructors *
 ************************************/
struct ast_node * ast_ctr_integer(int val);
struct ast_node * ast_ctr_binop(enum ast_binop_type op, struct ast_node * left, struct ast_node * right);
struct ast_node * ast_ctr_prnt(struct ast_node * subexpr);
struct ast_node * ast_ctr_block(struct ast_node * parent, struct ast_node * child);
struct ast_node * ast_ctr_punctuator(enum ast_punctuator_type type);
void ast_delete(struct ast_node ** root);


/************************
 * Presentation Methdos *
 ************************/
void ast_print_texttree(struct ast_node *root, FILE *strm);
void ast_print_dot(struct ast_node * root, FILE * strm);


#endif
// AST_H
