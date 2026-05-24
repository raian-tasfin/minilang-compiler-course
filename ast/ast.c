#include "ast.h"
#include "ast_kind.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/***************
 * AST Context *
 ***************/
struct ast_ctx
ast_ctx_init(struct cli_ast_opts opts)
{
    struct ast_ctx ctx = {0};
    if (opts.dot) {
        ctx.dot =
            opts.dot_path
            ? fopen(opts.dot_path, "w")
            : stdout;
        if (!ctx.dot) {
            fprintf(stderr, "Could not open '%s'\n", opts.dot_path);
            ctx.err = true;
            return ctx;
        }
    }
    if (opts.text) {
        ctx.text =
            opts.text_path
            ? fopen(opts.text_path, "w")
            : stdout;

        if (!ctx.text) {
            fprintf(stderr, "Could not open '%s'\n", opts.text_path);
            ctx.err = true;
            return ctx;
        }
    }
    return ctx;
}


/*************************
 * AST Node Constructors *
 *************************/
struct ast_node *
ast_ctr_integer(int val)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = AST_INTEGER,
        .integer = val
    };
    return node;
}


struct ast_node *
ast_ctr_binop(enum ast_binop_type op,
              struct ast_node * left,
              struct ast_node * right)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = AST_BINOP,
        .binop = {
            .op = op,
            .left = left,
            .right = right,
        }
    };
    return node;
}


struct ast_node *
ast_ctr_prnt(struct ast_node * subexpr)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = AST_PRNT,
        .print.child = subexpr,
    };
    return node;
}

struct ast_node *
ast_ctr_block(struct ast_node * parent, struct ast_node * child)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = AST_BLOCK,
        .block = {
            .parent = parent,
            .child = child,
        }
    };
    return node;
}

struct ast_node *
ast_ctr_punctuator(enum ast_punctuator_type type)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = AST_PUNCTUATOR,
        .punctuator = type,
    };
    return node;
}

/*********************
 * Text tree Drawing *
 *********************/
#define TT_BRANCH  "├── "
#define TT_LAST    "└── "
#define TT_VERT    "│   "
#define TT_BLANK   "    "

#define TT_MAX_DEPTH   256
#define TT_PREFIX_STEP 500

/* Count statements in a block chain */
static int
block_chain_len(struct ast_node * node)
{
    int n = 0;
    while (node && node->type == AST_BLOCK) {
        n++;
        node = node->block.parent;
    }
    return n;
}

static void
ast_print_texttree_r(struct ast_node *root,
                     FILE * strm,
                     char * prefix,
                     int  plen,
                     int  is_last);

/*
 * Print all statements in a block chain at the same indent level.
 * The chain is a singly-linked list via .parent; we recurse to reverse
 * it so statements print in source order (first stmt first).
 */
static void
block_chain_print(struct ast_node * node,
                  FILE * strm,
                  char * prefix,
                  int plen,
                  int remaining)
{
    if (!node || node->type != AST_BLOCK) return;

    /* recurse down the parent chain first (to get source order) */
    if (node->block.parent)
        block_chain_print(node->block.parent, strm, prefix, plen, remaining - 1);

    /* now print this node's child stmt */
    int is_last = (remaining == 1);
    ast_print_texttree_r(node->block.child, strm, prefix, plen, is_last);
}

static void
ast_print_texttree_r(struct ast_node *root,
                     FILE * strm,
                     char * prefix,
                     int  plen,
                     int  is_last)
{
    /**************
     * Base Cases *
     **************/
    if (!strm || !root) return;

    /*
     * BLOCK: don't print a node for the cons cell itself — just print
     * a "{block}" header then unroll the chain so all stmts appear as
     * siblings at the same indent level.
     */
    if (root->type == AST_BLOCK) {
        const char *connector = is_last ? TT_LAST : TT_BRANCH;
        fprintf(strm, "%s%s{block}\n", prefix, connector);

        const char *cont = is_last ? TT_BLANK : TT_VERT;
        int clen = (int) strlen(cont);
        if (plen + clen < TT_MAX_DEPTH * TT_PREFIX_STEP) {
            memcpy(prefix + plen, cont, clen);
            prefix[plen + clen] = '\0';
        }

        int n = block_chain_len(root);
        block_chain_print(root, strm, prefix, plen + clen, n);

        prefix[plen] = '\0';
        return;
    }

    /* print connector for this node */
    const char *connector = is_last ? TT_LAST : TT_BRANCH;
    fprintf(strm, "%s%s", prefix, connector);

    /**********************
     * Prefix Indentation *
     **********************/
    const char * cont = is_last ? TT_BLANK : TT_VERT;
    int clen = (int) strlen(cont);
    if (plen + clen < TT_MAX_DEPTH * TT_PREFIX_STEP) {
        memcpy(prefix + plen, cont, clen);
        prefix[plen + clen] = '\0';
    }

    /******************
     * Recursive Case *
     ******************/
    switch (root->type) {
    case AST_INTEGER:
        fprintf(strm, "%s: %d\n", astk_kind_to_str(AST_INTEGER), root->integer);
        break;
    case AST_BINOP:
        fprintf(strm, "%s: %s\n", astk_kind_to_str(AST_BINOP), astk_binop_to_str(root->binop.op));
        ast_print_texttree_r(root->binop.left,  strm, prefix, plen + clen, 0);
        ast_print_texttree_r(root->binop.right, strm, prefix, plen + clen, 1);
        break;
    case AST_PRNT:
        fprintf(strm, "%s\n", astk_kind_to_str(AST_PRNT));
        ast_print_texttree_r(root->print.child, strm, prefix, plen + clen, 1);
        break;
    case AST_PUNCTUATOR:
        fprintf(strm, "%s\n", astk_punc_to_str(root->punctuator));
        break;
    default:
        fprintf(strm, "%s\n", astk_kind_to_str(root->type));
        break;
    }

    prefix[plen] = '\0';
}

void
ast_print_texttree(struct ast_node *root, FILE *strm)
{
    if (!strm || !root) return;
    char prefix[TT_MAX_DEPTH * TT_PREFIX_STEP + 1];
    prefix[0] = '\0';
    ast_print_texttree_r(root, strm, prefix, 0, 1);
}


/**************
 * Dot Output *
 **************/

/*
 * Walk the block chain and emit each stmt as a direct child of
 * block_node_id, in source order.
 */
static int
ast_to_dot(struct ast_node * root,
           FILE * strm,
           int parent_id,
           int * dot_id);

static void
block_chain_dot(struct ast_node * node,
                FILE * strm,
                int block_node_id,
                int * dot_id)
{
    if (!node || node->type != AST_BLOCK) return;
    /* recurse first for source order */
    if (node->block.parent)
        block_chain_dot(node->block.parent, strm, block_node_id, dot_id);
    ast_to_dot(node->block.child, strm, block_node_id, dot_id);
}

static int
ast_to_dot(struct ast_node * root,
           FILE * strm,
           int parent_id,
           int * dot_id)
{
    /*************
     * Base Case *
     *************/
    if (!root || !strm) return -1;
    int my_id = dot_id[0]++;

    /*************************
     * Spawn Node with Label *
     *************************/
    switch (root->type) {
    case AST_INTEGER:
        fprintf(strm, "  node%d [label=\"INTEGER: %d\"];\n", my_id, root->integer);
        break;
    case AST_BINOP:
        fprintf(strm, "  node%d [label=\"BINOP: %s\"];\n", my_id, astk_binop_to_str(root->binop.op));
        break;
    case AST_PUNCTUATOR:
        fprintf(strm, "  node%d [label=\"PUNCTUATOR: %s\"];\n", my_id, astk_punc_to_str(root->punctuator));
        break;
    default:
        fprintf(strm, "  node%d [label=\"%s\"];\n", my_id, astk_kind_to_str(root->type));
    }

    /****************
     * Connect Edge *
     ****************/
    if (parent_id != -1) {
        fprintf(strm, "  node%d -> node%d;\n", parent_id, my_id);
    }

    /*************
     * Sub-nodes *
     *************/
    switch (root->type) {
    case AST_BINOP:
        ast_to_dot(root->binop.left, strm, my_id, dot_id);
        ast_to_dot(root->binop.right, strm, my_id, dot_id);
        break;
    case AST_PRNT:
        ast_to_dot(root->print.child, strm, my_id, dot_id);
        break;
    case AST_BLOCK:
        /* Unroll the chain: all stmts are direct children of this node */
        block_chain_dot(root, strm, my_id, dot_id);
        break;
    default:
        break;
    }
    return my_id;
}

void ast_print_dot(struct ast_node * root, FILE * strm)
{
    if (!strm || !root) return;
    int dot_id = 0;
    fprintf(strm, "digraph AST {\n");
    fprintf(strm, "  node [shape=box];\n");
    ast_to_dot(root, strm, -1, &dot_id);
    fprintf(strm, "}\n");
}


/******************
 * AST Destructor *
 ******************/
void
ast_delete(struct ast_node ** root)
{
    if (!root || !*root) return;
    switch ((*root)->type) {
    case AST_BINOP:
        ast_delete(&(*root)->binop.left);
        ast_delete(&(*root)->binop.right);
        break;
    case AST_PRNT:
        ast_delete(&(*root)->print.child);
        break;
    case AST_BLOCK:
        ast_delete(&(*root)->block.parent);
        ast_delete(&(*root)->block.child);
        break;
    default:
        break;
    }

    free(*root);
    *root = NULL;
}
