#include <stdio.h>
#include <stdbool.h>
#include "ast_kind.h"
#include "../cli/cli.h"
#include "../darr/darr.h"
#include "../symtable/symtable.h"

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
void ast_ctx_destroy(struct ast_ctx * ctx);


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
    enum scalar_type type;
    union {
        int integer;
        bool boolean;
    };
};

struct ast_ident_node {
    char * name;
    struct symbol * sym;
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
    struct sym_scope * scope;
};

struct ast_while_loop {
    struct ast_node * condition;
    struct ast_node * body;
};

struct ast_decl_node {
    char * name;
    struct symbol * sym;
    enum scalar_type type;
    struct ast_node * rhs;
};

struct ast_asn_node {
    char * name;
    struct symbol * sym;
    struct ast_node * rhs;
};


/*************
 * Condition *
 *************/
/* Things get easier if we wrap everything in ast_node's. Makes the
 * large control flow structures easier.
 */
struct ast_if_block {
    struct ast_node * condition;
    struct ast_node * body;
};

struct ast_elif_block {
    struct ast_node * condition;
    struct ast_node * body;
};

struct ast_else_block {
    struct ast_node * body;
};

struct ast_cond_stmt {
    struct ast_node * if_block;
    struct darr * elif_ladder;  // list of ast_node. Nodes are ast_elif type
    struct ast_node * else_block;
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
        struct ast_decl_node decl;
        struct ast_asn_node asn;
        struct ast_ident_node ident;
        struct ast_while_loop while_loop;
        // condition
        struct ast_cond_stmt cond_stmt;
        struct ast_if_block if_block;
        struct ast_elif_block elif_block;
        struct ast_else_block  else_block ;
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
ast_ctr_while_loop(struct ast_node * condition,
                   struct ast_node * body,
                   struct ast_node * current_block,
                   struct ast_src_loc loc);

struct ast_node *
ast_ctr_if_block(struct ast_node * condition,
                 struct ast_node * body,
                 struct ast_node * current_block,
                 struct ast_src_loc loc);

struct ast_node *
ast_ctr_elif_block(struct ast_node * condition,
                   struct ast_node * body,
                   struct ast_node * current_block,
                   struct ast_src_loc loc);

struct ast_node *
ast_ctr_else_block(struct ast_node * body,
                   struct ast_node * current_block,
                   struct ast_src_loc loc);


struct ast_node *
ast_ctr_cond_stmt(struct ast_node * if_block,
                  struct darr * elif_ladder,
                  struct ast_node * else_block,
                  struct ast_node * current_block,
                  struct ast_src_loc loc);

struct ast_node *
ast_ctr_block(struct ast_node * parent_block);

struct ast_node *
ast_ctr_punctuator(enum ast_punctuator_type type,
                   struct ast_node * current_block,
                   struct ast_src_loc loc);

struct ast_node *
ast_ctr_ident(char * name,
              struct ast_node * current_block,
              struct ast_src_loc loc);

struct ast_node *
ast_ctr_decl(enum scalar_type type,
             char * name,
             struct ast_node * expr,
             struct ast_node * current_block,
             struct ast_src_loc loc);

struct ast_node *
ast_ctr_asn(char * name,
            struct ast_node * expr,
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
