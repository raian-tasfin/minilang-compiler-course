#include <stddef.h>
#include "../ast/ast.h"

#ifndef INTERP_H
#define INTERP_H 1


enum ir_unit_type {
    IR_BLOCK,
    IR_STMT,
};

enum ir_stmt_type {
    IR_CONST_ASSIGNMENT,
    IR_BINOP_ASSIGNMENT,
    IR_PRINT,
};


/********************
 * Binary Operators *
 ********************/
enum ir_binop {
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_AND,
    IR_OR,
    IR_XOR,
};


/**********
 * Scalar *
 **********/
enum ir_scalar_type {
    IR_INTEGER,
    IR_BOOLEAN,
};

struct ir_scalar {
    enum ir_scalar_type type;
    union {
        bool boolean;
        int  integer;
    };
};


/**********
 * Symbol *
 **********/
struct ir_sym {
    enum ir_scalar_type type;
    char src_name[64];
    bool src_var;
    int  id;
};


/**************
 * Statements *
 **************/
struct ir_stmt_const_asn {
    struct ir_sym *  dest;
    struct ir_scalar scalar;
};

struct ir_stmt_binop_asn {
    enum ir_binop op;
    struct ir_sym * dest;
    struct ir_sym * val1;
    struct ir_sym * val2;
};

struct ir_stmt_print {
    struct ir_sym * val;
};

struct ir_stmt {
    enum ir_stmt_type type;
    int lineno;
    union {
        struct ir_stmt_const_asn const_asn;
        struct ir_stmt_binop_asn binop_asn;
        struct ir_stmt_print     print;
    };
};

struct ir_unit {
    enum ir_unit_type type;
    union {
        struct darr * block; // array of ir_units.
        struct ir_stmt stmt;
    };
};


/****************
 * IR Generator *
 ****************/
/**
 * An IR program is an array of units.
 */
struct ir_unit * // array of units
ir_prog_generate(struct ast_node * root);



/***********
 * Context *
 ***********/
struct ir_ctx {
    FILE * rprt;
    bool err;
};


struct ir_ctx
ir_ctx_init(struct cli_opts * cliopts);


void
ir_print(struct ir_ctx * ctx, struct ir_unit * prog);


#endif
// INTERP_H
