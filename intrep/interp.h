#include <inttypes.h>
#include <stddef.h>
#include "../ast/ast.h"
#include "../symtable/symtable.h"
#include "../bitset/bitset.h"

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
    IR_LABEL,
    IR_CJMP,
    IR_JMP,
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

struct ir_stmt_var_decl {
    struct symbol * sym;
};

struct ir_stmt_label {
    int id;
};

struct ir_stmt_cjmp {
    struct symbol * cond_symb;
    int loc_label;
};

struct ir_stmt_jmp {
    int loc_label;
};

struct ir_stmt {
    enum ir_stmt_type type;
    int lineno;
    union {
        struct ir_stmt_const_asn const_asn;
        struct ir_stmt_binop_asn binop_asn;
        struct ir_stmt_var_decl  decl;
        struct ir_stmt_unop_asn  unop_asn;
        struct ir_stmt_var_asn   var_asn;
        struct ir_stmt_print     print;
        struct ir_stmt_label     label;
        struct ir_stmt_cjmp      cjmp;
        struct ir_stmt_jmp       jmp;
    };
};

struct ir_block {
    struct darr * units; // array of ir_units
};

struct ir_unit {
    enum ir_unit_type type;

    struct bitset * use;
    struct bitset * def;
    struct bitset * in;
    struct bitset * out;

    struct darr * pred;  // array of predecessor units (ir_unit)
    struct darr * succ;  // array of successor units (ir_unit)

    union {
        struct ir_block block;
        struct ir_stmt stmt;
    };
};


/****************
 * IR Generator *
 ****************/
struct ir_prog
{
    struct ir_unit * root_unit;
    int cnt_lines;
    int cnt_labels;
};
struct ir_prog
ir_prog_generate(struct ast_node * root, struct sym_scope * scope);


/****************
 * CFG Analysis *
 ****************/
void
ir_cfg_analysis(struct ir_prog * prog, struct sym_scope * scope);


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
ir_ctx_destroy(struct ir_ctx * ctx);


void
ir_print(struct ir_ctx * ctx, struct ir_unit * prog);


#endif
// INTERP_H
