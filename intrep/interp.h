#include <inttypes.h>
#include <stddef.h>
#include "../ast/ast.h"
#include "../symtable/symtable.h"

#ifndef INTERP_H
#define INTERP_H 1


enum ir_unit_type {
    IR_BLOCK,
    IR_STMT,
};

enum ir_stmt_type {
    IR_CONST_ASSIGNMENT,
    IR_BINOP_ASSIGNMENT,
    IR_UNOP_ASSIGNMENT,
    IR_VAR_ASSIGNMENT,
    IR_VAR_DECL,
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
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_NE,
    IR_EQ,
};


/*******************
 * Unary Operators *
 *******************/
enum ir_unop {
    IR_NEG,
    IR_NOT,
};


/**********
 * Scalar *
 **********/
struct ir_scalar {
    enum scalar_type type;
    union {
        bool boolean;
        int  integer;
    };
};


/**************
 * Statements *
 **************/
struct ir_stmt_const_asn {
    struct symbol *  dest;
    struct ir_scalar scalar;
};

struct ir_stmt_binop_asn {
    enum ir_binop op;
    struct symbol * dest;
    struct symbol * val1;
    struct symbol * val2;
};

struct ir_stmt_unop_asn {
    enum ir_unop op;
    struct symbol * dest;
    struct symbol * val;
};

struct ir_stmt_var_asn {
    struct symbol * dest;
    struct symbol * val;
};

struct ir_stmt_print {
    struct symbol * val;
};

struct ir_var_decl {
    struct symbol * sym;
};

struct ir_stmt {
    enum ir_stmt_type type;
    int lineno;
    union {
        struct ir_stmt_const_asn const_asn;
        struct ir_stmt_binop_asn binop_asn;
        struct ir_stmt_unop_asn  unop_asn;
        struct ir_stmt_var_asn   var_asn;
        struct ir_stmt_print     print;
        struct ir_var_decl       decl;
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
ir_prog_generate(struct ast_node * root, struct sym_scope * scope);



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
