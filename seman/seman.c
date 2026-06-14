#include "seman.h"
#include "../symtable/symtable.h"
#include "../darr/darr.h"
#include <stdio.h>
#include <stdbool.h>


/*****************************
 * Semantic Analysis - Types *
 *****************************/
enum seman_type {
    SEMAN_NULL,
    SEMAN_BOOLEAN = SCAL_BOOLEAN,
    SEMAN_INTEGER = SCAL_INTEGER,
};

struct seman_report {
    enum seman_type type;
    bool err;
};

static struct seman_report
seman_report_err()
{
    return (struct seman_report) {
        .err = true
    };
}

static struct seman_report
seman_report_integer()
{
    return (struct seman_report) {
        .err = false,
        .type = SEMAN_INTEGER
    };
}

static struct seman_report
seman_report_booelan()
{
    return (struct seman_report) {
        .err = false,
        .type = SEMAN_BOOLEAN,
    };
}

static struct seman_report
seman_report_null()
{
    return (struct seman_report) {
        .err = false,
        .type = SEMAN_NULL,
    };
}


/*********************
 * Private Utilities *
 *********************/
static struct seman_report
seman_print_type_mismatch_error(struct ast_node * root,
                  char * expectation,
                  struct src_buffer * sb)
{
    fprintf(stderr, "%s. "AST_SRC_LOC_FMT".\n",
            expectation,
            AST_SRC_LOC_EXP(root->loc));
    src_buf_print_loc(sb, root->loc);
    return seman_report_err();
}

static struct seman_report
seman_print_unknown_identifier_error(struct ast_node * root,
                                     struct src_buffer * sb)
{
    fprintf(stderr, "Unknown variable.\n");
    src_buf_print_loc(sb, root->loc);
    return seman_report_err();
}

static struct seman_report
seman_print_duplicate_declaration_error(struct ast_node * root,
                                        struct src_buffer * sb)
{
    fprintf(stderr, "Variable previously declared.\n");
    src_buf_print_loc(sb, root->loc);
    return seman_report_err();
}

struct seman_report
seman_proc(struct ast_node * root,
           struct src_buffer * sb,
           struct sym_scope * current_scope,
           int * id)
{
    if (!root) return (struct seman_report){ .type = SEMAN_NULL };
    switch (root->type) {
    case AST_SCALAR:
        return (struct seman_report){
            .type = root->scalar.type
        };

    case AST_BINOP: {
        switch (root->binop.op) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD: {
            struct seman_report left = seman_proc(root->binop.left,
                                                  sb,
                                                  current_scope,
                                                  id);
            struct seman_report right = seman_proc(root->binop.right,
                                                   sb,
                                                   current_scope,
                                                   id);
            if (left.err || right.err) return seman_report_err();
            if (left.type != SEMAN_INTEGER)
                return seman_print_type_mismatch_error(root->binop.left,
                                                       "Expected integer",
                                                       sb);
            if (right.type != SEMAN_INTEGER)
                return seman_print_type_mismatch_error(root->binop.right,
                                                       "Expected integer",
                                                       sb);
            return seman_report_integer();
        }
        case AST_AND:
        case AST_OR:
        case AST_XOR: {
            struct seman_report left = seman_proc(root->binop.left,
                                                  sb,
                                                  current_scope,
                                                  id);
            struct seman_report right = seman_proc(root->binop.right,
                                                   sb,
                                                   current_scope,
                                                   id);
            if (left.err || right.err)
                return seman_report_err();
            if (left.type != SEMAN_BOOLEAN)
                return seman_print_type_mismatch_error(root->binop.left,
                                                       "Expected boolean",
                                                       sb);
            if (right.type != SEMAN_BOOLEAN)
                return
        seman_print_type_mismatch_error(root->binop.right,
                                        "Expected boolean",
                                        sb);
            return seman_report_booelan();
        }
        case AST_EQ:
        case AST_LE:
        case AST_GE:
        case AST_NE:
        case AST_LT:
        case AST_GT: {
            struct seman_report left = seman_proc(root->binop.left,
                                                  sb,
                                                  current_scope,
                                                  id);
            struct seman_report right = seman_proc(root->binop.right,
                                                   sb,
                                                   current_scope,
                                                   id);
            if (left.err || right.err) return seman_report_err();
            if (left.type != SEMAN_INTEGER)
                return seman_print_type_mismatch_error(root->binop.left,
                                                       "Expected integer",
                                                       sb);
            if (right.type != SEMAN_INTEGER)
                return seman_print_type_mismatch_error(root->binop.right,
                                                       "Expected integer",
                                                       sb);
            return seman_report_booelan();
        }
        default:
            return seman_print_type_mismatch_error(root, "Unknown binary operator", sb);
        }
    }
    case AST_UNOP: {
        switch (root->unop.op) {
        case AST_NEG: {
            struct seman_report child = seman_proc(root->unop.child,
                                                   sb,
                                                   current_scope,
                                                   id);
            if (child.err) return seman_report_err();
            if (child.type != SEMAN_INTEGER)
                return seman_print_type_mismatch_error(root->unop.child,
                                                       "Expected integer",
                                                       sb);
            return seman_report_integer();
        }
        case AST_NOT: {
            struct seman_report child = seman_proc(root->unop.child,
                                                   sb,
                                                   current_scope,
                                                   id);
            if (child.err) return seman_report_err();
            if (child.type != SEMAN_BOOLEAN)
                return seman_print_type_mismatch_error(root->unop.child,
                                                       "Expected boolean",
                                                       sb);
            return seman_report_booelan();
        }
        default:
            return seman_print_type_mismatch_error(root,
                                                   "Unknown unary operator",
                                                   sb);
        }
    }
    case AST_PRNT: {
        struct seman_report child = seman_proc(root->print.child,
                                               sb,
                                               current_scope,
                                               id);
        if (child.err) return seman_report_err();
        switch (child.type) {
        case SEMAN_INTEGER:
        case SEMAN_BOOLEAN:
            return seman_report_null();
        default: return seman_print_type_mismatch_error(root->print.child,
                                                        "Expected boolean or integer.",
                                                        sb);
        }
    }
    case AST_BLOCK: {
        struct sym_scope * scope = sym_scope_new(current_scope);
        root->block.scope = scope;
        int cnt_statements = darr_size(root->block.statements);
        for (int i = 0; i < cnt_statements; i++) {
            struct ast_node ** stmt = darr_get(root->block.statements, i);
            if (seman_proc(*stmt, sb, scope, id).err) return seman_report_err();
        }
        return seman_report_null();
    }
    case AST_WHILE_LOOP: {
        /* Ensure boolean condition */
        struct seman_report cond =
            seman_proc(root->while_loop.condition,
                       sb,
                       current_scope,
                       id);
        if (cond.err) return seman_report_err();
        if (cond.type != SEMAN_BOOLEAN)
            return seman_print_type_mismatch_error(root->unop.child,
                                                   "Expected boolean",
                                                   sb);
        /* Check block */
        struct seman_report body =
            seman_proc(root->while_loop.body,
                       sb,
                       current_scope,
                       id);
        if (body.err) return seman_report_err();

        return seman_report_null();
    }
    case AST_IDENT: {
        struct symbol * sym = sym_scope_find(current_scope, root->ident.name);
        if (!sym) return seman_print_unknown_identifier_error(root, sb);
        root->ident.sym = sym;
        return (struct seman_report){
            .type = sym_type(sym)
        };
    }
    case AST_ASSIGNMENT: {
        struct seman_report rhs = seman_proc(root->asn.rhs, sb, current_scope, id);
        if (rhs.err) return seman_report_err();

        struct symbol * sym = sym_scope_find(current_scope, root->asn.name);
        if (!sym) return seman_print_unknown_identifier_error(root, sb);
        root->asn.sym = sym;

        if (sym_type(sym) != rhs.type)
            return seman_print_type_mismatch_error(root, "Type mismatch.", sb);

        return (struct seman_report) {
            .type = rhs.type
        };
    }
    case AST_DECLARATION: {
        // check rhs
        struct seman_report rhs = { .type = SEMAN_NULL };
        if (root->decl.rhs) rhs = seman_proc(root->decl.rhs, sb, current_scope, id);

        // duplicate declaration check
        struct symbol * sym = sym_scope_find_local(current_scope, root->decl.name);
        if (sym) return seman_print_duplicate_declaration_error(root, sb);

        // get left type
        enum seman_type left_type = root->decl.type;

        // type mismatch
        if (rhs.type != SEMAN_NULL && left_type != rhs.type)
            return seman_print_type_mismatch_error(root, "Type mismatch.", sb);

        // install symbol
        sym = sym_new(current_scope,
                      root->decl.name,
                      left_type);
        root->decl.sym = sym;
        return (struct seman_report){ .type = left_type };
    }
    case AST_PUNCTUATOR: return seman_report_null();
    default: return seman_report_err();
    }
}


/**************
 * Public API *
 * *************/
bool
seman(struct ast_node * root, struct src_buffer * sb, struct sym_scope * scope)
{
    int id = 0;
    return !seman_proc(root, sb, scope, &id).err;
}
