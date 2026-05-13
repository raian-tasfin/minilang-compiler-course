#include "../symtable/symtable.h"
#include "../ast/ast.h"

#ifndef INTREP_H
#define INTREP_H 1


union arg {
    struct symrec * sym;
    int value;
};


struct ir_line {
    int type;
    struct symrec * dest;
    union arg arg1;
    union arg arg2;
};

struct ir_program {
    struct ir_line * lines;
    int size;
};

struct ir_program *
ir_generate(struct ast_node * root, struct scope * scope);

void
ir_print(struct ir_program * prog);

#endif
// INTREP_H
