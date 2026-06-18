#include "interp.h"
#include "../darr/darr.h"
#include "../boolean/boolean.h"
#include "../op/opstr.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>


static enum ir_binop
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

static enum ir_unop
ir_unop_from_ast(enum ast_unop_type op)
{
    switch (op) {
    case AST_NEG: return IR_NEG;
    case AST_NOT: return IR_NOT;
    }
}

static char *
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


static char *
ir_unopch(enum ir_unop op)
{
    switch (op) {
    case IR_NEG: return NEG_STR;
    case IR_NOT: return NOT_STR;
    }
}


/****************
 * Constructors *
 ****************/
static struct ir_block
ir_block_ctr(void)
{
    struct ir_block block = {
        .units = darr_init(sizeof(struct ir_unit)),
        .pred  = darr_init(sizeof(struct ir_block *)),
        .succ  = darr_init(sizeof(struct ir_block *)),
    };
    return block;
}
/****************
 * IR Generator *
 ****************/
static struct symbol *
ir_prog_generate_rec(struct ast_node * node,
                     struct ir_block * block,
                     struct sym_scope * scope,
                     int * lineno,
                     int * label);


// generate goto label statement block.
static struct ir_unit
ir_prog_generate_goto(int * lineno, int label_id)
{
    // create the block
    struct ir_unit block_unit = {
        .type = IR_BLOCK,
        .block = ir_block_ctr(),
    };
    // create the statement
    struct ir_unit goto_cond_unit = {
        .type = IR_STMT,
        .stmt = {
            .type = IR_JMP,
            .lineno = ++(*lineno),
            .jmp = { .loc_label = label_id, },
        },
    };
    // insert the statement in the wrapper
    darr_push_back(block_unit.block.units, &goto_cond_unit);
    return block_unit;
}

// generate conditional goto label statement block.
static struct ir_unit
ir_prog_generate_cond_goto(struct ast_node * condition,
                           int * lineno,
                           int * label,
                           struct sym_scope * parent_scope,
                           int condition_label_id,
                           int jump_label_id)
{
    /* separate block */
    struct ir_unit block = {
        .type = IR_BLOCK,
        .block = ir_block_ctr(),
    };
    /* seperate sub scope */
    struct sym_scope * scope = sym_scope_new(parent_scope);

    /* condition label statement */
    // construct
    struct ir_unit condition_label = {
        .type = IR_STMT,
        .stmt = {
            .type = IR_LABEL,
            .lineno = ++(*lineno),
            .label = { .id = condition_label_id, },
        },
    };
    // push
    darr_push_back(block.block.units, &condition_label);

    /* condition calculation */
    // recursively add calculation here
    struct symbol * cond_symb =
        ir_prog_generate_rec(condition,
                             &block.block,
                             scope,
                             lineno,
                             label);

    /* if condition goto block */
    // construct
    struct ir_unit cjmp = {
        .type = IR_STMT,
        .stmt = {
            .type = IR_CJMP,
            .lineno = ++(*lineno),
            .cjmp = {
                .cond_symb = cond_symb,
                .loc_label = jump_label_id,
            },
        },
    };
    darr_push_back(block.block.units, &cjmp);

    return block;
}

static struct symbol *
ir_prog_generate_rec(struct ast_node * node,
                     struct ir_block * block,
                     struct sym_scope * scope,
                     int * lineno,
                     int * label)
{
    switch (node->type) {
    case AST_SCALAR: {
        /* Create a const assignment */

        // create the scalar
        enum scalar_type type = node->scalar.type;
        struct ir_scalar scalar = { .type = type };
        switch (type) {
        case SCAL_INTEGER: scalar.integer = node->scalar.integer; break;
        case SCAL_BOOLEAN: scalar.boolean = node->scalar.boolean; break;
        }

        // create the smbol
        struct symbol * dest = sym_new(scope,
                                       NULL,
                                       node->scalar.type);

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

        darr_push_back(block->units, &unit);
        return dest;
    }
    case AST_BINOP: {
        /* Destination type */
        enum scalar_type dest_type;
        switch (node->binop.op) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
        case AST_MOD: {
            dest_type = SCAL_INTEGER;
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
            dest_type = SCAL_BOOLEAN;
            break;
        }
        }
        struct symbol * dest = sym_new(scope, NULL, dest_type);
        struct symbol * val1 = ir_prog_generate_rec(node->binop.left,
                                                    block,
                                                    scope,
                                                    lineno,
                                                    label);
        struct symbol * val2 = ir_prog_generate_rec(node->binop.right,
                                                    block,
                                                    scope,
                                                    lineno,
                                                    label);
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

        darr_push_back(block->units, &unit);
        return dest;
    }
    case AST_UNOP: {
        /* destination type */
        enum scalar_type dest_type;
        switch (node->unop.op) {
        case AST_NEG: dest_type = SCAL_INTEGER; break;
        case AST_NOT: dest_type = SCAL_BOOLEAN; break;
        }
        struct symbol * dest = sym_new(scope, NULL, dest_type);
        struct symbol * val = ir_prog_generate_rec(node->unop.child,
                                                   block,
                                                   scope,
                                                   lineno,
                                                   label);
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
        darr_push_back(block->units, &unit);
        return dest;
    }
    case AST_PRNT: {
        struct symbol * val = ir_prog_generate_rec(node->print.child,
                                                   block,
                                                   scope,
                                                   lineno,
                                                   label);
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
        darr_push_back(block->units, &unit);
        return NULL;
    }
    case AST_BLOCK: {
        /* generate new sub-block and attack a sub-scope to it */
        struct ir_block sub_block = ir_block_ctr();
        struct sym_scope * sub_scope = sym_scope_new(scope);

        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** subnode = darr_get(node->block.statements, i);
            ir_prog_generate_rec(*subnode,
                                 &sub_block,
                                 sub_scope,
                                 lineno,
                                 label);
        }

        // now we push that new block to the program
        struct ir_unit unit_block = {
            .type = IR_BLOCK,
            .block = sub_block,
        };
        darr_push_back(block->units, &unit_block);
        return NULL;
    }
    case AST_PUNCTUATOR: {
        return NULL;
    }
    case AST_ASSIGNMENT: {
        // rhs
        struct symbol * rhs = ir_prog_generate_rec(node->asn.rhs,
                                                   block,
                                                   scope,
                                                   lineno,
                                                   label);
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
        darr_push_back(block->units, &unit);
        return node->asn.sym;
    };
    case AST_DECLARATION: {
        /* Push declaration */
        struct ir_unit unit = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_VAR_DECL,
                .lineno = ++(*lineno),
                .decl = { .sym = node->decl.sym }
            }
        };
        darr_push_back(block->units, &unit);

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
                        .dest = sym_new(scope, NULL, node->decl.type),
                        .scalar = { .type = node->decl.type, }
                    }
                }
            };
            switch (node->decl.type) {
            case SCAL_BOOLEAN: unit.stmt.const_asn.scalar.boolean = false; break;
            case SCAL_INTEGER: unit.stmt.const_asn.scalar.integer = 0; break;
            }
            darr_push_back(block->units, &unit);
        } else {
            struct symbol * rhs =
                ir_prog_generate_rec(node->decl.rhs,
                                     block,
                                     scope,
                                     lineno,
                                     label);
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
            darr_push_back(block->units, &unit);
        }
        return node->decl.sym;
    }
    case AST_IDENT: {
        return node->ident.sym;
    }
    case AST_WHILE_LOOP: {
        /* Structure
         * ==============
         * We construct CFG as we go. So here each of the following
         * three are seperate blocks.  Those blocks are pushed at
         * current root block.
         *
         * -----------------
         * goto condition
         * -----------------
         * block:
         * ....
         * -----------------
         * block:
         * condition: if condition goto block
         * -----------------
         */

        int block_label_id = (*label)++;
        int condition_label_id = (*label)++;

        /* goto condition */
        struct ir_unit goto_cond = ir_prog_generate_goto(lineno, condition_label_id);


        /* sub_block includes block label */
        // construct sub_block
        struct ir_unit sub_block = {
            .type = IR_BLOCK,
            .block = ir_block_ctr(),
        };
        // construct label statement
        struct ir_unit block_label = {
            .type = IR_STMT,
            .stmt = {
                .type = IR_LABEL,
                .lineno = ++(*lineno),
                .label = { .id = block_label_id, },
            },
        };
        // push block label statement to sub_block
        darr_push_back(sub_block.block.units, &block_label);
        // the statements of that body belongs to block_unit
        // additionally need a new sub-scope
        struct sym_scope * sub_scope = sym_scope_new(scope);
        struct ast_node * body = node->while_loop.body;
        int body_n = darr_size(body->block.statements);
        for (int i = 0; i < body_n; i++) {
            struct ast_node ** child = darr_get(body->block.statements, i);
            ir_prog_generate_rec(*child,
                                 &sub_block.block,
                                 sub_scope,
                                 lineno,
                                 label);
        }

        // construct condition label statement unit
        struct ir_unit condition_block =
            ir_prog_generate_cond_goto(node->while_loop.condition,
                                       lineno,
                                       label,
                                       scope,
                                       condition_label_id,
                                       block_label_id);

        // push goto condition: block to unit
        darr_push_back(block->units, &goto_cond);
        // store index for reconnecting later
        int goto_cond_unit_indx = darr_size(block->units) - 1;
        // push the sub-block unit to root
        darr_push_back(block->units, &sub_block);
        // remember index, will be needed later to attach prev succ.
        int sub_block_indx = darr_size(block->units) - 1;
        // push the block to root
        darr_push_back(block->units, &condition_block);
        int condition_block_indx = darr_size(block->units) - 1;

        // now we collect the block pointers
        struct ir_unit * goto_block_ref = darr_get(block->units, goto_cond_unit_indx);
        struct ir_unit * sub_block_ref  = darr_get(block->units, sub_block_indx);
        struct ir_unit * cond_block_ref = darr_get(block->units, condition_block_indx);

        struct ir_block * goto_blk = &goto_block_ref->block;
        struct ir_block * sub_blk  = &sub_block_ref->block;
        struct ir_block * cond_blk = &cond_block_ref->block;

        // goto condition: -> condition_block
        darr_push_back(goto_blk->succ, &cond_blk);
        darr_push_back(cond_blk->pred, &goto_blk);

        // sub_block -> condition_block  (loop back edge)
        darr_push_back(sub_blk->succ,  &cond_blk);
        darr_push_back(cond_blk->pred, &sub_blk);

        // condition_block -> sub_block  (true branch)
        darr_push_back(cond_blk->succ, &sub_blk);
        darr_push_back(sub_blk->pred,  &cond_blk);

        return NULL;
    }
    };
}


struct ir_unit *
ir_prog_generate(struct ast_node * node, struct sym_scope * scope)
{
    if (!node || !scope) return NULL;
    struct ir_unit * root_unit = malloc(sizeof(struct ir_unit));
    *root_unit = (struct ir_unit) {
        .type = IR_BLOCK,
        .block = ir_block_ctr(),
    };
    int lineno = 0;
    int label = 0;
    ir_prog_generate_rec(node, &(root_unit->block), scope, &lineno, &label);
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
    case SCAL_INTEGER: {
        sprintf(dest, "%5d", scalar.integer);
        return;
    }
    case SCAL_BOOLEAN: {
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
        printf("{IR_BLOCK}\n");
        int n = darr_size(unit->block.units);
        for (int i = 0; i < n; i++) {
            struct ir_unit * subunit = darr_get(unit->block.units, i);
            ir_print(ctx, subunit);
        }
        return;
    }

    ir_fprintf(ctx, "%3d. ", unit->stmt.lineno);

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
                   "%7s = %7s %4s %7s\n",
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
                   "%7s = %7s %4s %7s\n",
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
        sprintf(type, "%s", scalar_type_to_str(unit->stmt.decl.sym->type));
        ir_fprintf(ctx,
                   "%7s   %7s      <%7s>\n",
                   "decl",
                   dest_name,
                   type);
        return;
    }
    case IR_LABEL: {
        char label[65];
        ir_fprintf(ctx,
                   "label_%d:\n",
                   unit->stmt.label.id);
        return;
    }
    case IR_CJMP: {
        char cond_name[65];
        char loc_name[65];
        ir_sym_to_str(unit->stmt.cjmp.cond_symb, cond_name);
        ir_fprintf(ctx,
                   "%7s   %7s %4s label_%d\n",
                   "if",
                   cond_name,
                   "goto",
                   unit->stmt.cjmp.loc_label);
        return;
    }
    case IR_JMP: {
        ir_fprintf(ctx,
                   "%7s   label_%d\n",
                   "goto",
                   unit->stmt.jmp.loc_label);
        return;
    }
    }
}

void
ir_ctx_destroy(struct ir_ctx * ctx)
{
    if (!ctx) return;
    if (ctx->rprt
        && ctx->rprt != stdout
        && ctx->rprt != stdin
        && ctx->rprt != stderr)
        fclose(ctx->rprt);
}
