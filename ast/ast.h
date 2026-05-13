#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../cli/cli.h"
#include "../parser/parser.tab.h"

#ifndef AST_H
#define AST_H 1

struct ast_ctx {
    FILE * dot;
    FILE * text;
    bool err;
};

struct ast_ctx ast_ctx_init(struct cli_ast_opts opts);

struct ast_node {
    int token_type;

    /* Value union */
    union {
        int INTEGER;
    } value;

    /* Child pointers */
    union {
        struct {
            struct ast_node * left;
            struct ast_node * right;
        };
        struct ast_node * child;
    };
};

struct ast_node * ast_ctr_integer(int val);
struct ast_node * ast_ctr_subexpr(struct ast_node * subexpr);

struct ast_node *
ast_ctr_binop(int op_type,
              struct ast_node * left,
              struct ast_node * right);

void ast_print_postorder(struct ast_node * root, FILE * strm);
void ast_print_texttree(struct ast_node * root, FILE * strm);
void ast_print_dot(struct ast_node * root, FILE * strm);
void ast_delete(struct ast_node ** root);
int ast_cnt_nodes(struct ast_node * root);

#endif
// AST_H
