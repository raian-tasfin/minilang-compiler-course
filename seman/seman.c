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
static enum seman_type
seman_print_error(struct ast_node * root,
                  char * expectation,
                  struct src_buffer * sb)
{
    fprintf(stderr, "%s. "AST_SRC_LOC_FMT".\n",
            expectation,
            AST_SRC_LOC_EXP(root->loc));
    src_buf_print_loc(sb, root->loc);
    return SEMAN_ERR;
}


enum seman_type
seman_type_check_proc(struct ast_node * root, struct src_buffer * sb)
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
            enum seman_type left = seman_type_check_proc(root->binop.left, sb);
            enum seman_type right = seman_type_check_proc(root->binop.right, sb);
            if (left == SEMAN_ERR || right == SEMAN_ERR) return SEMAN_ERR;
            if (left != SEMAN_INTEGER) return seman_print_error(root->binop.left, "Expected integer", sb);
            if (right != SEMAN_INTEGER) return seman_print_error(root->binop.right, "Expected integer", sb);
            return SEMAN_INTEGER;
        }
        case AST_AND:
        case AST_OR:
        case AST_XOR: {
            enum seman_type left = seman_type_check_proc(root->binop.left, sb);
            enum seman_type right = seman_type_check_proc(root->binop.right, sb);
            if (left == SEMAN_ERR || right == SEMAN_ERR) return SEMAN_ERR;
            if (left != SEMAN_BOOLEAN)
                return seman_print_error(root->binop.left, "Expected boolean", sb);
            if (right != SEMAN_BOOLEAN)
                return seman_print_error(root->binop.right, "Expected boolean", sb);
            return SEMAN_BOOLEAN;
        }
        case AST_EQ:
        case AST_LE:
        case AST_GE:
        case AST_NE:
        case AST_LT:
        case AST_GT: {
            enum seman_type left = seman_type_check_proc(root->binop.left, sb);
            enum seman_type right = seman_type_check_proc(root->binop.right, sb);
            if (left == SEMAN_ERR || right == SEMAN_ERR) return SEMAN_ERR;
            if (left != SEMAN_INTEGER)
                return seman_print_error(root->binop.left, "Expected integer", sb);
            if (right != SEMAN_INTEGER)
                return seman_print_error(root->binop.right, "Expected integer", sb);
            return SEMAN_BOOLEAN;
        }
        default:
            return seman_print_error(root, "Unknown binary operator", sb);
        }
    }
    case AST_UNOP: {
        switch (root->unop.op) {
        case AST_NEG: {
            enum seman_type child = seman_type_check_proc(root->unop.child, sb);
            if (child == SEMAN_ERR) return SEMAN_ERR;
            if (child != SEMAN_INTEGER) return seman_print_error(root->unop.child, "Expected integer", sb);
            return SEMAN_INTEGER;
        }
        case AST_NOT: {
            enum seman_type child = seman_type_check_proc(root->unop.child, sb);
            if (child == SEMAN_ERR) return SEMAN_ERR;
            if (child != SEMAN_BOOLEAN) return seman_print_error(root->unop.child, "Expected boolean", sb);
            return SEMAN_BOOLEAN;
        }
        default:
            return seman_print_error(root, "Unknown unary operator", sb);
        }
    }
    case AST_PRNT: {
        enum seman_type type = seman_type_check_proc(root->print.child, sb);
        switch (type) {
        case SEMAN_INTEGER:
        case SEMAN_BOOLEAN:
        case SEMAN_ERR: return type;
        default: return seman_print_error(root->print.child, "Expected boolean or integer.", sb);
        }
    }
    case AST_BLOCK: {
        int cnt_statements = darr_size(root->block.statements);
        for (int i = 0; i < cnt_statements; i++) {
            struct ast_node ** stmt = darr_get(root->block.statements, i);
            if (seman_type_check_proc(*stmt, sb) == SEMAN_ERR) return SEMAN_ERR;
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
seman_type_check(struct ast_node * root, struct src_buffer * sb)
{
    return seman_type_check_proc(root, sb) != SEMAN_ERR;
}
