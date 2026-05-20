#include <stddef.h>
#include "../ast/ast.h"

#ifndef _H
#define _H 1

enum ir_type {
    IR_INTEGER,
};

enum ir_stmt_type {
    IR_CONST_ASSIGNMENT,
    IR_BINOP_ASSIGNMENT,
    IR_PRINT,
};

enum ir_binop {
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
};

struct ir_sym {
    char src_name[64];
    bool src_var;
    int id;
    enum ir_type type;
};



struct ir_arg {
    enum ir_type type;
    struct ir_sym * sym;
};

struct ir_stmt_const_assignment {
    struct ir_sym * dest;
    int val;
};

struct ir_stmt_binop_assignment {
    enum ir_binop op;
    struct ir_sym * dst;
    struct ir_arg arg1;
    struct ir_arg arg2;
};

struct ir_stmt_prnt {
    struct ir_arg arg;
};

struct ir_stmt {
    enum ir_stmt_type type;
    union {
        struct ir_stmt_const_assignment const_asn;
        struct ir_stmt_binop_assignment binop_asn;
        struct ir_stmt_prnt prnt;
    };
};

struct ir_block {
    struct ir_stmt * stmts;
    int size;
    int cap;
};

struct ir_block *
ir_block_generate(struct ast_node * root);

struct ir_ctx {
    FILE * rprt;
    bool err;
};

struct ir_ctx
ir_ctx_init(struct cli_opts * cliopts);

void
ir_print(struct ir_ctx * ctx, struct ir_block * block);

#endif
// _H
