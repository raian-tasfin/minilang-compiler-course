#include "ast.h"
#include "ast_kind.h"
#include <stdint.h>
#include <stdio.h>

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
        .value.INTEGER = val,
        .left = NULL,
        .right = NULL
    };
    return node;
}

struct ast_node *
ast_ctr_binop(enum ast_kind op_type,
              struct ast_node * left,
              struct ast_node * right)
{
    struct ast_node * node = malloc(sizeof(struct ast_node));
    if (!node) {
        fprintf(stderr, "Failed allocating node.\n");
        return NULL;
    }
    node[0] = (struct ast_node) {
        .type = op_type,
        .left = left,
        .right = right
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
#define TT_PREFIX_STEP 4

static void
ast_print_texttree_r(struct ast_node *root,
                     FILE * strm,
                     char * prefix,
                     int  plen,
                     int  is_last)
{
    if (!strm || !root) return;
    const char *connector = is_last ? TT_LAST : TT_BRANCH;
    fprintf(strm, "%s%s", prefix, connector);
    switch (root->type) {
    case AST_INTEGER:
        fprintf(strm, "%s: %d\n", astk_tokstr(AST_INTEGER), root->value.INTEGER);
        return;
    default:
        fprintf(strm, "%s\n", astk_tokstr(root->type));
        break;
    }
    switch (root->type) {
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD: {
        const char * cont = is_last ? TT_BLANK : TT_VERT;
        int clen = (int) strlen(cont);
        if (plen + clen < TT_MAX_DEPTH * TT_PREFIX_STEP) {
            memcpy(prefix + plen, cont, clen);
            prefix[plen + clen] = '\0';
        }
        ast_print_texttree_r(root->left,  strm, prefix, plen + clen, 0);
        ast_print_texttree_r(root->right, strm, prefix, plen + clen, 1);
        prefix[plen] = '\0';
        break;
    }
    default:
        break;
    }
}

void
ast_print_texttree(struct ast_node *root, FILE *strm)
{
    if (!strm || !root) return;
    char prefix[TT_MAX_DEPTH * 6 + 1];
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
    if (root->type == AST_INTEGER) {
        fprintf(strm,
                "  node%d [label=\"INTEGER: %d\"];\n",
                my_id,
                root->value.INTEGER);
    } else {
        fprintf(strm,
                "  node%d [label=\"%s\"];\n",
                my_id,
                astk_tokstr(root->type));
    }
    if (parent_id != -1) {
        fprintf(strm, "  node%d -> node%d;\n", parent_id, my_id);
    }
    switch (root->type) {
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
        ast_to_dot(root->left, strm, my_id, dot_id);
        ast_to_dot(root->right, strm, my_id, dot_id);
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



void
ast_delete(struct ast_node ** root)
{
    if (!root) return;
    if (!*root) return;

    if (root[0]->child) ast_delete(&root[0]->child);
    if (root[0]->left) ast_delete(&root[0]->left);
    if (root[0]->right) ast_delete(&root[0]->right);

    free(root[0]);
    root[0] = NULL;
}


int
ast_cnt_nodes(struct ast_node * root)
{
    if (!root) return 0;
    switch (root->type) {
    case AST_INTEGER:   return 1;
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD: return 1 + (ast_cnt_nodes(root->left)
                              + ast_cnt_nodes(root->right));
    }

    return 0;
}
