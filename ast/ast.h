#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ast_kind.h"
#include "../cli/cli.h"
#include "../darr/darr.h"

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
struct ast_src_loc {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
};
#define AST_SRC_LOC_FMT "%d:%d-%d:%d"
#define AST_SRC_LOC_EXP(loc)     \
    (loc).first_line,            \
        (loc).first_column,      \
        (loc).last_line,         \
        (loc).last_column


struct ast_scalar_node {
    enum ast_scalar_type type;
    union {
        int integer;
        bool boolean;
    };
};

struct ast_binop_node {
    enum ast_binop_type op;
    struct ast_node * left;
    struct ast_node * right;
};

struct ast_unop_node {
    enum ast_unop_type op;
    struct ast_node * child;
};

struct ast_print_node {
    struct ast_node * child;
};

struct ast_block_node {
    struct darr * statements;       // array of ast_node's
    struct ast_node * parent_block;
};

struct ast_node {
    enum ast_kind type;
    struct ast_node * current_block;
    struct ast_src_loc loc;
    union {
        struct ast_scalar_node scalar;
        struct ast_binop_node binop;
        struct ast_unop_node unop;
        struct ast_print_node print;
        struct ast_block_node block;
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
struct ast_node *
ast_ctr_integer(int val,
                struct ast_node * current_block,
                struct ast_src_loc loc);

struct ast_node *
ast_ctr_boolean(bool val,
                struct ast_node * current_block,
                struct ast_src_loc loc);

struct ast_node *
ast_ctr_binop(enum ast_binop_type op,
              struct ast_node * left,
              struct ast_node * right,
              struct ast_node * current_block,
              struct ast_src_loc loc);

struct ast_node *
ast_ctr_unop(enum ast_unop_type op,
             struct ast_node * child,
             struct ast_node * current_block,
             struct ast_src_loc loc);


struct ast_node *
ast_ctr_prnt(struct ast_node * subexpr,
             struct ast_node * current_block,
             struct ast_src_loc loc);

struct ast_node *
ast_ctr_block(struct ast_node * parent_block);

struct ast_node *
ast_ctr_punctuator(enum ast_punctuator_type type,
                   struct ast_node * current_block,
                   struct ast_src_loc loc);

void ast_delete(struct ast_node ** root);
void ast_finalize(struct ast_node * root);


/************************
 * Presentation Methdos *
 ************************/
void ast_print_texttree(struct ast_node *root, FILE *strm);
void ast_print_dot(struct ast_node * root, FILE * strm);

#endif
// AST_H
