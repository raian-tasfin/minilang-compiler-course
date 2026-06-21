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
 * IR Generator *
 ****************/
static struct symbol *
ir_prog_generate_rec(struct ast_node * node,
                     struct ir_prog * prog,
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
        struct ir_stmt stmt = {
                .type = IR_CONST_ASSIGNMENT,
                .lineno = ++(*lineno),
                .const_asn = {
                    .dest = dest,
                    .scalar = scalar,
                }
            };
        darr_push_back(prog->stmts, &stmt);
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
                                                    prog,
                                                    scope,
                                                    lineno,
                                                    label);
        struct symbol * val2 = ir_prog_generate_rec(node->binop.right,
                                                    prog,
                                                    scope,
                                                    lineno,
                                                    label);

        struct ir_stmt stmt = {
                .type = IR_BINOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .binop_asn = {
                    .op = ir_binop_from_ast(node->binop.op),
                    .dest = dest,
                    .val1 = val1,
                    .val2 = val2,
                }
            };

        darr_push_back(prog->stmts, &stmt);
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
                                                   prog,
                                                   scope,
                                                   lineno,
                                                   label);
        struct ir_stmt stmt = {
                .type = IR_UNOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .unop_asn = {
                    .op = ir_unop_from_ast(node->unop.op),
                    .dest = dest,
                    .val  = val,
                }
            };
        darr_push_back(prog->stmts, &stmt);
        return dest;
    }
    case AST_PRNT: {
        struct symbol * val = ir_prog_generate_rec(node->print.child,
                                                   prog,
                                                   scope,
                                                   lineno,
                                                   label);
        struct ir_stmt stmt = {
                .type = IR_PRINT,
                .lineno = ++(*lineno),
                .print = {
                    .val = val
                },
            };

        darr_push_back(prog->stmts, &stmt);
        return NULL;
    }
    case AST_BLOCK: {
        /* This is like a subprogram. So we need a subscope */
        struct sym_scope * subscope = sym_scope_new(scope);
        /* add subnodes */
        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** subnode = darr_get(node->block.statements, i);
            ir_prog_generate_rec(*subnode,
                                 prog,
                                 subscope,
                                 lineno,
                                 label);
        }
        return NULL;
    }
    case AST_PUNCTUATOR: {
        return NULL;
    }
    case AST_ASSIGNMENT: {
        // rhs
        struct symbol * rhs = ir_prog_generate_rec(node->asn.rhs,
                                                   prog,
                                                   scope,
                                                   lineno,
                                                   label);
        // statement
        struct ir_stmt stmt = {
                .type = IR_VAR_ASSIGNMENT,
                .lineno = ++(*lineno),
                .var_asn = {
                    .dest = node->asn.sym,
                    .val = rhs,
                }
        };

        darr_push_back(prog->stmts, &stmt);
        return node->asn.sym;
    };
    case AST_DECLARATION: {
        /* Push declaration */
        struct ir_stmt stmt = {
                .type = IR_VAR_DECL,
                .lineno = ++(*lineno),
                .decl = { .sym = node->decl.sym }
        };

        darr_push_back(prog->stmts, &stmt);

        /* Push assignment
         * const assignment of default value if none provided
         * var assignment otherwise
         */
        if (!node->decl.rhs) {
            struct ir_stmt stmt = {
                    .type = IR_CONST_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .const_asn = {
                        .dest = sym_new(scope, NULL, node->decl.type),
                        .scalar = { .type = node->decl.type, }
                    }
                };

            switch (node->decl.type) {
            case SCAL_BOOLEAN: stmt.const_asn.scalar.boolean = false; break;
            case SCAL_INTEGER: stmt.const_asn.scalar.integer = 0; break;
            }
            darr_push_back(prog->stmts, &stmt);
        } else {
            struct symbol * rhs =
                ir_prog_generate_rec(node->decl.rhs,
                                     prog,
                                     scope,
                                     lineno,
                                     label);
            struct ir_stmt stmt = {
                    .type = IR_VAR_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .var_asn = {
                        .dest = node->decl.sym,
                        .val = rhs,
                    }
                };
            darr_push_back(prog->stmts, &stmt);
        }
        return node->decl.sym;
    }
    case AST_IDENT: {
        return node->ident.sym;
    }
    case AST_WHILE_LOOP: {
        /* Structure
         * ==============
         * goto condition
         * block:
         * ....
         * condition: if condition goto block
         * -----------------
         */
        int cond_label = ++(prog->cnt_labels);
        int block_label = ++(prog->cnt_labels);

        /* goto condition */
        struct ir_stmt goto_condition = {
            .type = IR_JMP,
            .lineno = ++(*lineno),
            .jmp.loc_label = cond_label,
        };
        darr_push_back(prog->stmts, &goto_condition);

        /* block: */
        struct ir_stmt block_label_stmt = {
            .type = IR_LABEL,
            .lineno = ++(*lineno),
            .label.id = block_label,
        };
        darr_push_back(prog->stmts, &block_label_stmt);

        /* loop body */
        /* Loop body to condition is a new scope */
        struct sym_scope * subscope = sym_scope_new(scope);
        struct ast_node * body = node->while_loop.body;
        ir_prog_generate_rec(body,
                             prog,
                             subscope,
                             lineno,
                             label);

        /* condition: */
        struct ir_stmt cond_label_stmt = {
            .type = IR_LABEL,
            .lineno = ++(*lineno),
            .label.id = cond_label,
        };
        darr_push_back(prog->stmts, &cond_label_stmt);

        /* condition body */
        struct ast_node * condition = node->while_loop.condition;
        struct symbol * condition_sym = ir_prog_generate_rec(condition,
                                                             prog,
                                                             subscope,
                                                             lineno,
                                                             label);
        struct ir_stmt cond_cjmp = {
            .type = IR_CJMP,
            .lineno = ++(*lineno),
            .cjmp = {
                .cond_symb = condition_sym,
                .loc_label = block_label,
            },
        };
        darr_push_back(prog->stmts, &cond_cjmp);
        return NULL;
    }
    case AST_COND: {
        /* Structure
         * *********
         * if   (C1) B1
         * elif (C2) B2
         * elif (C3) B3
         * elif (C4) B4
         * else      B5
         *
         * Transform to
         * ************
         *
         * compute C1
         * compute C2
         * compute C3
         * compute C4
         *
         * if  C1 goto L1
         * if  C2 goto L2
         * if  C3 goto L3
         * if  C4 goto L4
         *
         * L1:
         *   B1
         *   goto L6
         * L2:
         *   B2
         *   goto L6
         * L3:
         *   B1
         *   goto L6
         * L4:
         *   B4
         *   goto L6
         * L5:
         *   B5
         *   goto L6
         * L6:
         */

        int cnt_conds = darr_size(node->cond_stmt.if_ladder);

        /* label calculation */
        /* labels are calculated as
         *     label_base + label_indx + 1
         */
        int exit_label_indx =
            cnt_conds                               // conditional blocks
            + (node->cond_stmt.else_block ? 1 : 0); // else generates a block if present
        int label_base = *label;
        /* We need to ensure the global label is out of this range,
         * i.e. we need to reserve this  range of labels. This is
         * because nested blocks may need more labels.
         */
        *label = label_base + exit_label_indx + 1 + 1;

        /* compute symbols */
        struct darr * cond_symbs = darr_init(sizeof(struct symbol));
        darr_reserve(cond_symbs, cnt_conds);
        for (int i = 0; i < cnt_conds; i++) {
            struct ast_node * if_child = darr_get(node->cond_stmt.if_ladder, i);
            darr_push_back(cond_symbs, ir_prog_generate_rec(if_child->if_block.condition, prog, scope, lineno, label));
        }

        /* generate lines of the form
         *   if C_i goto L_i
         */
        for (int i = 0; i < cnt_conds; i++) {
            struct symbol * cond_sym = darr_get(cond_symbs, i);
            struct ir_stmt cjmp = {
                .type = IR_CJMP,
                .lineno = ++(*lineno),
                .cjmp = {
                    .cond_symb = cond_sym,
                    .loc_label = label_base + i + 1,
                },
            };
            darr_push_back(prog->stmts, &cjmp);
        }

        /* generate blocks */
        /* if and elif blocks first */
        for (int i = 0; i < cnt_conds; i++) {
            // label statement
            struct ir_stmt stmt_label = {
                .type = IR_LABEL,
                .lineno = ++(*lineno),
                .label.id = label_base + i + 1,
            };
            darr_push_back(prog->stmts, &stmt_label);

            // block
            struct ast_node * child_if = darr_get(node->cond_stmt.if_ladder, i);
            ir_prog_generate_rec(child_if->if_block.body, prog, scope, lineno, label);

            // goto exit label
            struct ir_stmt goto_exit = {
                .type = IR_JMP,
                .lineno = ++(*lineno),
                .label.id = label_base + exit_label_indx + 1,
            };
            darr_push_back(prog->stmts, &goto_exit);
        }

        /* now else block if it exists */
        if (node->cond_stmt.else_block) {
            // label statement
            struct ir_stmt stmt_label = {
                .type = IR_LABEL,
                .lineno = ++(*lineno),
                .label.id = label_base + cnt_conds + 1,
            };
            darr_push_back(prog->stmts, &stmt_label);
            ir_prog_generate_rec(node->cond_stmt.else_block->else_block.body, prog, scope, lineno, label);
        }

        /* finally exit label */
        struct ir_stmt stmt_label = {
            .type = IR_LABEL,
            .lineno = ++(*lineno),
            .label.id = label_base + exit_label_indx + 1,
        };
        darr_push_back(prog->stmts, &stmt_label);

        return NULL;
    }
    };
}


struct ir_prog
ir_prog_generate(struct ast_node * node, struct sym_scope * scope)
{
    if (!node || !scope) {
        return (struct ir_prog ){
            .scope = NULL,
            .cnt_labels = 0,
            .cnt_lines = 0,
            .stmts = NULL,
        };
    }

    struct ir_prog prog = {
        .scope = scope,
        .stmts = darr_init(sizeof(struct ir_stmt)),
    };
    ir_prog_generate_rec(node,
                         &prog,
                         prog.scope,
                         &(prog.cnt_lines),
                         &(prog.cnt_labels));
    return prog;
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
ir_print_rec(struct ir_ctx * ctx, struct ir_stmt * stmt)
{
    ir_fprintf(ctx, "%3d. ", stmt->lineno);

    switch (stmt->type) {
    case IR_CONST_ASSIGNMENT: {
        char dest_name[65];
        char scalar_string[65];
        ir_sym_to_str(stmt->const_asn.dest, dest_name);
        ir_scalar_to_str(stmt->const_asn.scalar, scalar_string);
        ir_fprintf(ctx, "%7s = %7s\n", dest_name, scalar_string);
        return;
    }
    case IR_BINOP_ASSIGNMENT: {
        char dest_name[65];
        char val1[65];
        char val2[65];
        ir_sym_to_str(stmt->binop_asn.dest, dest_name);
        ir_sym_to_str(stmt->binop_asn.val1, val1);
        ir_sym_to_str(stmt->binop_asn.val2, val2);
        ir_fprintf(ctx,
                   "%7s = %7s %4s %7s\n",
                   dest_name,
                   val1,
                   ir_binopch(stmt->binop_asn.op),
                   val2);
        return;
    }
    case IR_UNOP_ASSIGNMENT: {
        char dest_name[65];
        char val[65];
        ir_sym_to_str(stmt->unop_asn.dest, dest_name);
        ir_sym_to_str(stmt->unop_asn.val, val);
        ir_fprintf(ctx,
                   "%7s = %7s %4s %7s\n",
                   dest_name,
                   "",
                   ir_unopch(stmt->unop_asn.op),
                   val);
        return;
    }
    case IR_PRINT: {
        char val_str[65];
        ir_sym_to_str(stmt->print.val, val_str);
        ir_fprintf(ctx, "%7s   %7s\n", "print", val_str);
        return;
    }
    case IR_VAR_ASSIGNMENT: {
        char dest_name[65];
        char val[65];
        ir_sym_to_str(stmt->var_asn.dest, dest_name);
        ir_sym_to_str(stmt->var_asn.val, val);
        ir_fprintf(ctx,
                   "%7s = %7s\n",
                   dest_name,
                   val);
        return;
    }
    case IR_VAR_DECL: {
        char dest_name[65];
        char type[65];
        ir_sym_to_str(stmt->decl.sym, dest_name);
        sprintf(type, "%s", scalar_type_to_str(stmt->decl.sym->type));
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
                   stmt->label.id);
        return;
    }
    case IR_CJMP: {
        char cond_name[65];
        char loc_name[65];
        ir_sym_to_str(stmt->cjmp.cond_symb, cond_name);
        ir_fprintf(ctx,
                   "%7s   %7s %4s label_%d\n",
                   "if",
                   cond_name,
                   "goto",
                   stmt->cjmp.loc_label);
        return;
    }
    case IR_JMP: {
        ir_fprintf(ctx,
                   "%7s   label_%d\n",
                   "goto",
                   stmt->jmp.loc_label);
        return;
    }
    }
}


void
ir_print(struct ir_ctx * ctx, struct ir_prog * prog)
{
    if (!ctx || !ctx->rprt || ctx->err || !prog || !prog->stmts) return;
    int n = darr_size(prog->stmts);
    for (int i = 0; i < n; i++) {
        struct ir_stmt * stmt = darr_get(prog->stmts, i);
        ir_print_rec(ctx, stmt);
    }
    return;
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
