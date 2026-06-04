#include "seman.h"
#include "../darr/darr.h"
#include <stdio.h>


/*****************************
 * Semantic Analysis - Types *
 *****************************/
enum seman_type {
    SEMAN_NULL,
    SEMAN_ERR,
    SEMAN_BOOLEAN,
    SEMAN_INTEGER,
};


enum seman_type
seman_from_ast_scalar(enum ast_scalar_type type)
{
    switch (type) {
    case AST_BOOLEAN: return SEMAN_BOOLEAN;
    case AST_INTEGER: return SEMAN_INTEGER;
    default: return SEMAN_ERR;
    }
}


/*********************
 * Private Utilities *
 *********************/
enum seman_type
seman_print_error(struct ast_node * root, char * expectation)
{
    if (!root || !expectation) {
        fprintf(stderr, "Erroneous call to seman_print_error.\n");
        return SEMAN_ERR;
    }
    fprintf(stderr,
            "%s. "AST_SRC_LOC_FMT". Found '%s'\n",
            expectation,
            AST_SRC_LOC_EXP(root->loc),
            root->src
            );
    return SEMAN_ERR;
}


enum seman_type
seman_type_check_proc(struct ast_node * root)
{
    if (!root) return SEMAN_NULL;
    switch (root->type) {
    case AST_SCALAR:
        return seman_from_ast_scalar(root->scalar.type);

    case AST_BINOP: {
        switch (root->binop.op) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD: {
            enum seman_type left = seman_type_check_proc(root->binop.left);
            enum seman_type right = seman_type_check_proc(root->binop.right);
            if (left == SEMAN_ERR || right == SEMAN_ERR) return SEMAN_ERR;
            if (left != SEMAN_INTEGER) return seman_print_error(root->binop.left, "Expected integer");
            if (right != SEMAN_INTEGER) return seman_print_error(root->binop.right, "Expected integer");
            return SEMAN_INTEGER;
        }
        case AST_AND:
        case AST_OR:
        case AST_XOR: {
            enum seman_type left = seman_type_check_proc(root->binop.left);
            enum seman_type right = seman_type_check_proc(root->binop.right);
            if (left == SEMAN_ERR || right == SEMAN_ERR) return SEMAN_ERR;
            if (left != SEMAN_BOOLEAN)
                return seman_print_error(root->binop.left, "Expected boolean");
            if (right != SEMAN_BOOLEAN)
                return seman_print_error(root->binop.right, "Expected boolean");
            return SEMAN_BOOLEAN;
        }
        default:
            return seman_print_error(root, "Unknown binary operator");
        }
    }
    case AST_PRNT: {
        enum seman_type type = seman_type_check_proc(root->print.child);
        switch (type) {
        case SEMAN_INTEGER:
        case SEMAN_BOOLEAN:
        case SEMAN_ERR: return type;
        default: return seman_print_error(root->print.child, "Expected boolean or integer.");
        }
    }
    case AST_BLOCK: {
        printf("calling from block\n");
        int cnt_statements = darr_size(root->block.statements);
        for (int i = 0; i < cnt_statements; i++) {
            struct ast_node ** stmt = darr_get(root->block.statements, i);
            if (seman_type_check_proc(*stmt) == SEMAN_ERR) return SEMAN_ERR;
        }
        return SEMAN_NULL;
    }
    case AST_PUNCTUATOR: return SEMAN_NULL;
    default: return SEMAN_ERR;
    }
}


/**************
 * Public API *
 * *************/
bool
seman_type_check(struct ast_node * root)
{
    return seman_type_check_proc(root) != SEMAN_ERR;
}
