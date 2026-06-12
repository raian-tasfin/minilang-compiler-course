#include "ast.h"
#include "ast_kind.h"
#include "../darr/darr.h"
#include "../boolean/boolean.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*******************
 * Error Reporting *
 *******************/
static void
ast_print_src(struct ast_src_loc loc)
{
    fprintf(stderr, "ine %d, col %d-%d\n",
            loc.first_line,
            loc.first_column,
            loc.last_column);
}


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
ast_ctr_integer(int val,
                struct ast_node * current_block,
                struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    *node = (struct ast_node) {
        .type = AST_SCALAR,
        .current_block = current_block,
        .loc = loc,
        .scalar = {
            .type = AST_INTEGER,
            .integer = val,
        }
    };

    return node;
error:
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_boolean(bool val,
                struct ast_node * current_block,
                struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    *node = (struct ast_node) {
        .type = AST_SCALAR,
        .current_block = current_block,
        .loc = loc,
        .scalar = {
            .type = AST_BOOLEAN,
            .boolean = val,
        }
    };

    return node;
error:
    if (node) free(node);
    return NULL;
}


struct ast_node *
ast_ctr_binop(enum ast_binop_type op,
              struct ast_node * left,
              struct ast_node * right,
              struct ast_node * current_block,
              struct ast_src_loc loc)
{
    struct ast_node * node = NULL;

    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_BINOP,
        .current_block = current_block,
        .loc = loc,
        .binop = {
            .op = op,
            .left = left,
            .right = right,
        }
    };
    return node;

error:
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_unop(enum ast_unop_type op,
             struct ast_node * child,
             struct ast_node * current_block,
             struct ast_src_loc loc)
{
    struct ast_node * node = NULL;

    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_UNOP,
        .current_block = current_block,
        .loc = loc,
        .unop = {
            .op = op,
            .child = child,
        }
    };
    return node;

error:
    if (node) free(node);
    return NULL;
}


struct ast_node *
ast_ctr_prnt(struct ast_node * subexpr,
             struct ast_node * current_block,
             struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_PRNT,
        .current_block = current_block,
        .loc = loc,
        .print.child = subexpr,
    };
    return node;

 error:
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_block(struct ast_node * parent_block)
{
    struct ast_node * node = NULL;

    if (!(node = malloc(sizeof(struct ast_node))))  {
        fprintf(stderr, "Failed allocating node.\n");
        goto error;
    }
    node[0] = (struct ast_node){
        .type = AST_BLOCK,
        .current_block = NULL,
        .block.parent_block = parent_block
    };
    if (!(node->block.statements = darr_init(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating statement array.\n");
        goto error;
    }

    return node;
error:
    if (node && node->block.statements) darr_destroy(&node->block.statements);
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_punctuator(enum ast_punctuator_type type,
                   struct ast_node * current_block,
                   struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_PUNCTUATOR,
        .current_block = current_block,
        .loc = loc,
        .punctuator = type,
    };
    return node;

error:
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_ident(char * name,
              struct ast_node * current_block,
              struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_IDENT,
        .current_block = current_block,
        .loc = loc,
        .ident = {
            .name = name
        },
    };
    return node;

error:
    if (node) free(node);
    return NULL;
}


struct ast_node *
ast_ctr_decl(enum ast_scalar_type type,
             char * name,
             struct ast_node * expr,
             struct ast_node * current_block,
             struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_DECLARATION,
        .current_block = current_block,
        .loc = loc,
        .decl = {
            .name = name,
            .type = type,
            .rhs = expr,
        },
    };
    return node;
error:
    if (node) free(node);
    return NULL;
}

struct ast_node *
ast_ctr_asn(char * name,
            struct ast_node * expr,
            struct ast_node * current_block,
            struct ast_src_loc loc)
{
    struct ast_node * node = NULL;
    if (!(node = malloc(sizeof(struct ast_node)))) {
        fprintf(stderr, "Failed allocating node.\n");
        ast_print_src(loc);
        goto error;
    }

    node[0] = (struct ast_node) {
        .type = AST_DECLARATION,
        .current_block = current_block,
        .loc = loc,
        .asn = {
            .name = name,
            .rhs = expr,
        },
    };
    return node;
error:
    if (node) free(node);
    return NULL;
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

static void
ast_print_texttree_r(struct ast_node *root,
                     FILE * strm,
                     char * prefix,
                     int  plen,
                     int  is_last);

static void
ast_print_texttree_r(struct ast_node *root,
                     FILE * strm,
                     char * prefix,
                     int  plen,
                     int  is_last)
{
    if (!strm || !root) return;

    const char *connector = is_last ? TT_LAST : TT_BRANCH;

    if (root->type == AST_BLOCK) {
        fprintf(strm, "%s%s{block}\n", prefix, connector);

        const char *cont = is_last ? TT_BLANK : TT_VERT;
        int clen = (int) strlen(cont);
        if (plen + clen < TT_MAX_DEPTH * TT_PREFIX_STEP) {
            memcpy(prefix + plen, cont, clen);
            prefix[plen + clen] = '\0';
        }

        int n = darr_size(root->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node **stmt = darr_get(root->block.statements, i);
            ast_print_texttree_r(*stmt, strm, prefix, plen + clen, i == n - 1);
        }

        prefix[plen] = '\0';
        return;
    }

    fprintf(strm, "%s%s", prefix, connector);

    const char *cont = is_last ? TT_BLANK : TT_VERT;
    int clen = (int) strlen(cont);
    if (plen + clen < TT_MAX_DEPTH * TT_PREFIX_STEP) {
        memcpy(prefix + plen, cont, clen);
        prefix[plen + clen] = '\0';
    }

    switch (root->type) {
    case AST_SCALAR:
        switch (root->scalar.type) {
        case AST_INTEGER:
            fprintf(strm, "%s: %d\n", astk_scalar_to_str(AST_INTEGER), root->scalar.integer);
            break;
        case AST_BOOLEAN:
            fprintf(strm, "%s: %s\n", astk_scalar_to_str(AST_BOOLEAN), bool_to_str(root->scalar.boolean));
            break;
        }
        break;
    case AST_BINOP:
        fprintf(strm, "%s: %s\n", astk_kind_to_str(AST_BINOP), astk_binop_to_str(root->binop.op));

        ast_print_texttree_r(root->binop.left,  strm, prefix, plen + clen, 0);
        ast_print_texttree_r(root->binop.right, strm, prefix, plen + clen, 1);
        break;
    case AST_UNOP:
        fprintf(strm, "%s: %s\n", astk_kind_to_str(AST_UNOP), astk_unop_to_str(root->unop.op));

        ast_print_texttree_r(root->unop.child,  strm, prefix, plen + clen, 0);
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

static int
ast_to_dot(struct ast_node * root,
           FILE * strm,
           int parent_id,
           int * dot_id)
{
    if (!root || !strm) return -1;
    int my_id = dot_id[0]++;

    switch (root->type) {
    case AST_SCALAR:
        switch (root->scalar.type) {
        case AST_INTEGER:
            fprintf(strm, "  node%d [label=\"INTEGER: %d\"];\n", my_id, root->scalar.integer);
            break;
        case AST_BOOLEAN:
            fprintf(strm, "  node%d [label=\"BOOLEAN: %s\"];\n", my_id, bool_to_str(root->scalar.boolean));
            break;
        }
        break;
    case AST_BINOP:
        fprintf(strm, "  node%d [label=\"BINOP: %s\"];\n", my_id, astk_binop_to_str(root->binop.op));
        break;
    case AST_UNOP:
        fprintf(strm, "  node%d [label=\"UNOP: %s\"];\n", my_id, astk_unop_to_str(root->unop.op));
        break;
    case AST_PUNCTUATOR:
        fprintf(strm, "  node%d [label=\"PUNCTUATOR: %s\"];\n", my_id, astk_punc_to_str(root->punctuator));
        break;
    default:
        fprintf(strm, "  node%d [label=\"%s\"];\n", my_id, astk_kind_to_str(root->type));
        break;
    }

    if (parent_id != -1)
        fprintf(strm, "  node%d -> node%d;\n", parent_id, my_id);

    switch (root->type) {
    case AST_BINOP:
        ast_to_dot(root->binop.left,  strm, my_id, dot_id);
        ast_to_dot(root->binop.right, strm, my_id, dot_id);
        break;
    case AST_UNOP:
        ast_to_dot(root->unop.child,  strm, my_id, dot_id);
        break;
    case AST_PRNT:
        ast_to_dot(root->print.child, strm, my_id, dot_id);
        break;
    case AST_BLOCK: {
        int n = darr_size(root->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node **stmt = darr_get(root->block.statements, i);
            ast_to_dot(*stmt, strm, my_id, dot_id);
        }
        break;
    }
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
    case AST_SCALAR: break;
    case AST_BINOP:
        ast_delete(&(*root)->binop.left);
        ast_delete(&(*root)->binop.right);
        break;
    case AST_UNOP:
        ast_delete(&(*root)->unop.child);
        break;
    case AST_PRNT:
        ast_delete(&(*root)->print.child);
        break;
    case AST_BLOCK: {
        int n = darr_size((*root)->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** stmt_ptr = darr_get((*root)->block.statements, i);
            ast_delete(stmt_ptr);
        }
        darr_destroy(&(*root)->block.statements);
        break;
    }
    case AST_PUNCTUATOR:
        break;
    }
    free(*root);
    *root = NULL;
}


/****************
 * Finalize AST *
 ****************/
/* - update parent_block and current_block.
 */
static void
ast_finalize_r(struct ast_node * node,
               struct ast_node * containing_block)
{
    // base case
    if (!node) return;

    // block node
    if (node->type == AST_BLOCK) {
        node->current_block = NULL;
        node->block.parent_block = containing_block;

        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node **stmt_ptr = darr_get(node->block.statements, i);
            ast_finalize_r(*stmt_ptr, node);
        }
        return;
    }

    // general enclosure
    node->current_block = containing_block;

    // non-block nodes
    switch (node->type) {
    case AST_BINOP:
        ast_finalize_r(node->binop.left,  containing_block);
        ast_finalize_r(node->binop.right, containing_block);
        break;
    case AST_UNOP:
        ast_finalize_r(node->unop.child,  containing_block);
        break;
    case AST_PRNT:
        ast_finalize_r(node->print.child, containing_block);
        break;
    case AST_SCALAR:
    case AST_PUNCTUATOR:
    case AST_IDENT:
    default:
        break;
    }
}

void
ast_finalize(struct ast_node * root)
{
    ast_finalize_r(root, NULL);
}
