#include "interp.h"
#include "../darr/darr.h"
#include "../boolean/boolean.h"
#include "../op/opstr.h"
#include <stdarg.h>
#include <stdio.h>

enum ir_scalar_type
ir_scalar_type_from_ast(enum ast_scalar_type type)
{
    switch (type) {
    case AST_BOOLEAN: return IR_BOOLEAN;
    case AST_INTEGER: return IR_INTEGER;
    }
}

enum ir_binop
ir_binop_from_ast(enum ast_binop_type op)
{
    switch (op) {
    case AST_ADD: return IR_ADD;
    case AST_SUB: return IR_SUB;
    case AST_MUL: return IR_MUL;
    case AST_DIV: return IR_DIV;
    case AST_MOD: return IR_MOD;
    case AST_AND: return IR_AND;
    case AST_OR : return IR_OR;
    case AST_XOR: return IR_XOR;
    case AST_LT : return IR_LT;
    case AST_LE : return IR_LE;
    case AST_GT : return IR_GT;
    case AST_GE : return IR_GE;
    case AST_NE : return IR_NE;
    case AST_EQ : return IR_EQ;
    }
}

enum ir_unop
ir_unop_from_ast(enum ast_unop_type op)
{
    switch (op) {
    case AST_NEG: return IR_NEG;
    case AST_NOT: return IR_NOT;
    }
}

char *
ir_binopch(enum ir_binop op)
{
    switch (op) {
    case IR_ADD: return ADD_STR;
    case IR_SUB: return SUB_STR;
    case IR_MUL: return MUL_STR;
    case IR_DIV: return DIV_STR;
    case IR_MOD: return MOD_STR;
    case IR_AND: return AND_STR;
    case IR_OR : return OR_STR;
    case IR_XOR: return XOR_STR;
    case IR_LT : return LT_STR;
    case IR_LE : return LE_STR;
    case IR_GT : return GT_STR;
    case IR_GE : return GE_STR;
    case IR_NE : return NE_STR;
    case IR_EQ : return EQ_STR;
    }
}


char *
ir_unopch(enum ir_unop op)
{
    switch (op) {
    case IR_NEG: return NEG_STR;
    case IR_NOT: return NOT_STR;
    }
}



struct ir_sym *
ir_sym_new(char * name, enum ir_scalar_type type, int * var_id)
{
    struct ir_sym * new = malloc(sizeof(struct ir_sym));
    new[0] = (struct ir_sym){
        .id = var_id[0]++,
        .type = type
    };
    if (name) {
        strncpy(new->src_name, name, 63);
        new->src_var = true;
    }
    return new;
}


/****************
 * IR Generator *
 ****************/
static struct ir_sym *
ir_prog_generate_rec(struct ir_unit * root_unit,
                     struct ast_node * node,
                     int * var_id,
                     int * lineno)
{
    switch (node->type) {
    case AST_SCALAR: {
        /* Create a const assignment */

        // create the scalar
        enum ir_scalar_type type = ir_scalar_type_from_ast(node->scalar.type);
        struct ir_scalar scalar = { .type = type };
        if (type == IR_INTEGER) scalar.integer = node->scalar.integer;
        else scalar.boolean = node->scalar.boolean;

        // create the smbol
        struct ir_sym * dest = ir_sym_new(NULL, type, var_id);

        // tmp_i = scalar value
        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_CONST_ASSIGNMENT,
                .lineno = ++(*lineno),
                .const_asn = {
                    .dest = dest,
                    .scalar = scalar,
                }
            }
        };

        darr_push_back(root_unit->block, &unit);
        return dest;
    }
    case AST_BINOP: {
        /* Destination type */
        enum ir_scalar_type dest_type;
        switch (node->binop.op) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD: {
            dest_type = IR_INTEGER;
            break;
        }
        case AST_LT:
        case AST_LE:
        case AST_GT:
        case AST_GE:
        case AST_NE:
        case AST_EQ:
        case AST_AND:
        case AST_OR:
        case AST_XOR: {
            dest_type = IR_BOOLEAN;
            break;
        }
        }
        struct ir_sym * dest = ir_sym_new(NULL, dest_type, var_id);
        struct ir_sym * val1 = ir_prog_generate_rec(root_unit, node->binop.left, var_id, lineno);
        struct ir_sym * val2 = ir_prog_generate_rec(root_unit, node->binop.right, var_id, lineno);

        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_BINOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .binop_asn = {
                    .op = ir_binop_from_ast(node->binop.op),
                    .dest = dest,
                    .val1 = val1,
                    .val2 = val2,
                }
            },
        };

        darr_push_back(root_unit->block, &unit);
        return dest;
    }
    case AST_UNOP: {
        /* destination type */
        enum ir_scalar_type dest_type;
        switch (node->unop.op) {
        case AST_NEG: dest_type = IR_INTEGER; break;
        case AST_NOT: dest_type = IR_BOOLEAN; break;
        }
        struct ir_sym * dest = ir_sym_new(NULL, dest_type, var_id);
        struct ir_sym * val = ir_prog_generate_rec(root_unit, node->unop.child, var_id, lineno);

        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_UNOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .unop_asn = {
                    .op = ir_unop_from_ast(node->unop.op),
                    .dest = dest,
                    .val  = val,
                }
            },
        };

        darr_push_back(root_unit->block, &unit);
        return dest;
    }
    case AST_PRNT: {
        struct ir_sym * val = ir_prog_generate_rec(root_unit, node->print.child, var_id, lineno);
        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_PRINT,
                .lineno = ++(*lineno),
                .print = {
                    .val = val
                },
            },
        };
        darr_push_back(root_unit->block, &unit);
        return NULL;
    }
    case AST_BLOCK: {
        // block is like a subprogram. we first generate the block
        struct ir_unit block = {
            .type = IR_BLOCK,
            .block = darr_init(sizeof(struct ir_unit)),
        };
        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** subnode = darr_get(node->block.statements, i);
            ir_prog_generate_rec(&block, *subnode, var_id, lineno);
        }

        // now we push that block to the program
        darr_push_back(root_unit->block, &block);
        return NULL;
    }
    case AST_PUNCTUATOR: {
        return NULL;
    }
    };
}


struct ir_unit *
ir_prog_generate(struct ast_node * root)
{
    if (!root) return NULL;
    int var_id = 0;
    int lineno = 0;

    struct ir_unit * root_unit = malloc(sizeof(struct ir_unit));
    *root_unit = (struct ir_unit){
        .type = IR_BLOCK,
        .block = darr_init(sizeof(struct ir_unit)),
    };
    ir_prog_generate_rec(root_unit, root, &var_id, &lineno);
    return root_unit;
}


void
ir_fprintf(struct ir_ctx * ctx, const char *fmt, ...)
{
    if (!ctx || !ctx->rprt) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->rprt, fmt, args);
    va_end(args);
}


struct ir_ctx
ir_ctx_init(struct cli_opts * cliopts)
{
    struct ir_ctx ctx = { 0 };
    if (!cliopts) return ctx;

    /**************
     * Report Ctx *
     **************/
    if (!cliopts->ir.rprt) return ctx;
    if (!cliopts->ir.path) ctx.rprt = stdout;
    else ctx.rprt = fopen(cliopts->ir.path, "w");
    if (!ctx.rprt) {
        fprintf(stderr,
                "Could not open file '%s'\n",
                cliopts->ir.path);
        ctx.err = true;
    }

    return ctx;
}


void
ir_sym_to_str(struct ir_sym * sym, char * dest)
{
    if (sym->src_var) {
        sprintf(dest, "%s", sym->src_name);
        return;
    }
    sprintf(dest, "tmp_%d", sym->id);
}


void
ir_scalar_to_str(struct ir_scalar scalar, char * dest)
{
    switch (scalar.type) {
    case IR_INTEGER: {
        sprintf(dest, "%5d", scalar.integer);
        return;
    }
    case IR_BOOLEAN: {
        sprintf(dest, "%5s", bool_to_str(scalar.boolean));
        return;
    }
    }
}


void
ir_print(struct ir_ctx * ctx, struct ir_unit * unit)
{
    if (!ctx || !ctx->rprt || ctx->err || !unit) return;
    if (unit->type == IR_BLOCK) {
        int n = darr_size(unit->block);
        for (int i = 0; i < n; i++) {
            struct ir_unit * subunit = darr_get(unit->block, i);
            ir_print(ctx, subunit);
        }
        return;
    }

    printf("%3d. ", unit->stmt.lineno);

    switch (unit->stmt.type) {
    case IR_CONST_ASSIGNMENT: {
        char dest_name[65];
        char scalar_string[65];
        ir_sym_to_str(unit->stmt.const_asn.dest, dest_name);
        ir_scalar_to_str(unit->stmt.const_asn.scalar, scalar_string);
        ir_fprintf(ctx, "%7s = %7s\n", dest_name, scalar_string);
        return;
    }
    case IR_BINOP_ASSIGNMENT: {
        char dest_name[65];
        char val1[65];
        char val2[65];
        ir_sym_to_str(unit->stmt.binop_asn.dest, dest_name);
        ir_sym_to_str(unit->stmt.binop_asn.val1, val1);
        ir_sym_to_str(unit->stmt.binop_asn.val2, val2);
        ir_fprintf(ctx,
                   "%7s = %7s %3s %7s\n",
                   dest_name,
                   val1,
                   ir_binopch(unit->stmt.binop_asn.op),
                   val2);
        return;
    }
    case IR_UNOP_ASSIGNMENT: {
        char dest_name[65];
        char val[65];
        ir_sym_to_str(unit->stmt.unop_asn.dest, dest_name);
        ir_sym_to_str(unit->stmt.unop_asn.val, val);
        ir_fprintf(ctx,
                   "%7s = %7s %3s %7s\n",
                   dest_name,
                   "",
                   ir_unopch(unit->stmt.unop_asn.op),
                   val);
        return;
    }
    case IR_PRINT: {
        char val_str[65];
        ir_sym_to_str(unit->stmt.print.val, val_str);
        ir_fprintf(ctx, "%7s   %7s\n", "print", val_str);
        return;
    }
    }
}
