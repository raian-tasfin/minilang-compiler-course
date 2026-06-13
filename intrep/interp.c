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

enum sym_scalar_type
sym_scalar_type_from_ast(enum ast_scalar_type type)
{
    switch (type) {
    case AST_BOOLEAN: return SYM_BOOLEAN;
    case AST_INTEGER: return SYM_INTEGER;
    }
}


enum sym_scalar_type
sym_scalar_type_from_ir(enum ir_scalar_type type)
{
    switch (type) {
    case IR_BOOLEAN: return SYM_BOOLEAN;
    case IR_INTEGER: return SYM_INTEGER;
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


/****************
 * IR Generator *
 ****************/
static struct symbol *
ir_prog_generate_rec(struct ir_unit * root_unit,
                     struct ast_node * node,
                     int * lineno,
                     struct sym_scope * scope)
{
    switch (node->type) {
    case AST_SCALAR: {
        /* Create a const assignment */

        // create the scalar
        enum ir_scalar_type type = ir_scalar_type_from_ast(node->scalar.type);
        struct ir_scalar scalar = { .type = type };
        // create the smbol
        struct symbol * dest = sym_new(scope,
                                       NULL,
                                       sym_scalar_type_from_ast(node->scalar.type));

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
        struct symbol * dest = sym_new(scope, NULL, sym_scalar_type_from_ir(dest_type));
        struct symbol * val1 = ir_prog_generate_rec(root_unit,
                                                    node->binop.left,
                                                    lineno,
                                                    scope);
        struct symbol * val2 = ir_prog_generate_rec(root_unit,
                                                    node->binop.right,
                                                    lineno,
                                                    scope);
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
        struct symbol * dest = sym_new(scope, NULL, sym_scalar_type_from_ir(dest_type));
        struct symbol * val = ir_prog_generate_rec(root_unit,
                                                   node->unop.child,
                                                   lineno,
                                                   scope);
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
        struct symbol * val = ir_prog_generate_rec(root_unit,
                                                   node->print.child,
                                                   lineno,
                                                   scope);
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
        // attach scope to block node
        struct sym_scope * block_scope = node->block.scope;
        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** subnode = darr_get(node->block.statements, i);
            ir_prog_generate_rec(&block,
                                 *subnode,
                                 lineno,
                                 block_scope);
        }

        // now we push that block to the program
        darr_push_back(root_unit->block, &block);
        return NULL;
    }
    case AST_PUNCTUATOR: {
        return NULL;
    }
    case AST_ASSIGNMENT: {
        // rhs
        struct symbol * rhs = ir_prog_generate_rec(root_unit,
                                                   node->asn.rhs,
                                                   lineno,
                                                   scope);
        // statement
        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_VAR_ASSIGNMENT,
                .lineno = ++(*lineno),
                .var_asn = {
                    .dest = node->asn.sym,
                    .val = rhs,
                }
            }
        };
        darr_push_back(root_unit->block, &unit);
        return node->asn.sym;
    };
    case AST_DECLARATION: {
        /* Push declaration */
        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_VAR_DECL,
                .lineno = ++(*lineno),
                .decl = {
                    .sym = node->decl.sym
                }
            }
        };
        darr_push_back(root_unit->block, &unit);

        /* Push assignment
         * const assignment of default value if none provided
         * var assignment otherwise
         */
        if (!node->decl.rhs) {
            struct ir_unit unit = {
                .type = IR_STMT,
                .stmt = {
                    .type = IR_CONST_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .const_asn = {
                        .dest = sym_new(scope, NULL, sym_scalar_type_from_ast(node->decl.type)),
                        .scalar = {
                            .type = ir_scalar_type_from_ast(node->decl.type),
                        }
                    }
                }
            };
            switch (node->decl.type) {
            case AST_BOOLEAN: unit.stmt.const_asn.scalar.boolean = false; break;
            case AST_INTEGER: unit.stmt.const_asn.scalar.integer = 0; break;
            }
            darr_push_back(root_unit->block, &unit);
        } else {
            struct symbol * rhs =
                ir_prog_generate_rec(root_unit,
                                     node->decl.rhs,
                                     lineno,
                                     scope);
            struct ir_unit unit = {
                .type = IR_STMT,
                .stmt = {
                    .type = IR_VAR_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .var_asn = {
                        .dest = node->decl.sym,
                        .val = rhs,
                    }
                }
            };
            darr_push_back(root_unit->block, &unit);
        }

        return node->decl.sym;
    }
    case AST_IDENT: {
        return node->ident.sym;
    }
    }
}


struct ir_unit *
ir_prog_generate(struct ast_node * root, struct sym_scope * scope)
{
    if (!root) return NULL;
    int lineno = 0;

    struct ir_unit * root_unit = malloc(sizeof(struct ir_unit));
    *root_unit = (struct ir_unit){
        .type = IR_BLOCK,
        .block = darr_init(sizeof(struct ir_unit)),
    };
    ir_prog_generate_rec(root_unit, root, &lineno, scope);
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
ir_sym_to_str(struct symbol * sym, char * dest)
{
    if (sym->name) {
        sprintf(dest, "%s", sym->name);
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
    case IR_VAR_ASSIGNMENT: {
        char dest_name[65];
        char val[65];
        ir_sym_to_str(unit->stmt.var_asn.dest, dest_name);
        ir_sym_to_str(unit->stmt.var_asn.val, val);
        ir_fprintf(ctx,
                   "%7s = %7s\n",
                   dest_name,
                   val);
        return;
    }
    case IR_VAR_DECL: {
        char dest_name[65];
        char type[65];
        ir_sym_to_str(unit->stmt.decl.sym, dest_name);
        sprintf(type, "%s", sym_scalar_type_to_str(unit->stmt.decl.sym->type));
        ir_fprintf(ctx,
                   "%7s   %7s     <%7s>\n",
                   "decl",
                   dest_name,
                   type);
        return;
    }
    }
}
