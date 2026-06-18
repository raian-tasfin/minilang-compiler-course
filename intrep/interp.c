#include "interp.h"
#include "../darr/darr.h"
#include "../boolean/boolean.h"
#include "../op/opstr.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

static void
ir_wire_sequential(struct ir_unit * unit);


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
static struct ir_unit
ir_unit_ctr(enum ir_unit_type type)
{
    struct ir_unit unit = {
        .type = type,
        .pred = darr_init(sizeof(struct ir_unit *)),
        .succ = darr_init(sizeof(struct ir_unit *)),
    };
    return unit;
}

static struct ir_block
ir_block_ctr(void)
{
    return (struct ir_block){
        .units = darr_init(sizeof(struct ir_unit)),
    };
}

static struct ir_unit
ir_unit_block_ctr(void)
{
    struct ir_unit unit = ir_unit_ctr(IR_BLOCK);
    unit.block = ir_block_ctr();
    return unit;
}

static struct ir_unit
ir_unit_stmt_ctr(struct ir_stmt stmt)
{
    struct ir_unit unit = ir_unit_ctr(IR_STMT);
    unit.stmt = stmt;
    return unit;
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


// add goto label: unit and return the index
static int
ir_prog_add_goto(int * lineno, int label_id, struct ir_block * block)
{
    // create statement
    struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
            .type = IR_JMP,
            .lineno = ++(*lineno),
            .jmp = { .loc_label = label_id, },
        });
    // push to block
    darr_push_back(block->units, &unit);
    // return index
    return darr_size(block->units) - 1;
}

// add conditional jump to block and return the index of the CJMP statement.
static int
ir_prog_add_cond_goto(struct ast_node * condition,
                      int * lineno,
                      int * label,
                      struct sym_scope * scope,
                      int condition_label_id,
                      int jump_label_id,
                      struct ir_block * block)
{
    struct ir_unit condition_label = ir_unit_stmt_ctr((struct ir_stmt){
            .type = IR_LABEL,
            .lineno = ++(*lineno),
            .label = { .id = condition_label_id, },
        });
    // push
    darr_push_back(block->units, &condition_label);
    int cond_label_indx = darr_size(block->units) - 1;

    /* condition calculation */
    // recursively add calculation here
    struct symbol * cond_symb =
        ir_prog_generate_rec(condition,
                             block,
                             scope,
                             lineno,
                             label);
    /* if condition goto block */
    // construct
    struct ir_unit cjmp = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_CJMP,
                .lineno = ++(*lineno),
                .cjmp = {
                    .cond_symb = cond_symb,
                    .loc_label = jump_label_id,
                },
            });
    darr_push_back(block->units, &cjmp);
    /* Return the index of the CJMP (last pushed), not the label.
     * Callers need the CJMP to wire succ edges for liveness. */
    return darr_size(block->units) - 1;
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
        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_CONST_ASSIGNMENT,
                .lineno = ++(*lineno),
                .const_asn = {
                    .dest = dest,
                    .scalar = scalar,
                }
            });
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

        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_BINOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .binop_asn = {
                    .op = ir_binop_from_ast(node->binop.op),
                    .dest = dest,
                    .val1 = val1,
                    .val2 = val2,
                }
            });

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
        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_UNOP_ASSIGNMENT,
                .lineno = ++(*lineno),
                .unop_asn = {
                    .op = ir_unop_from_ast(node->unop.op),
                    .dest = dest,
                    .val  = val,
                }
            });
        darr_push_back(block->units, &unit);
        return dest;
    }
    case AST_PRNT: {
        struct symbol * val = ir_prog_generate_rec(node->print.child,
                                                   block,
                                                   scope,
                                                   lineno,
                                                   label);
        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_PRINT,
                .lineno = ++(*lineno),
                .print = {
                    .val = val
                },
            });
        darr_push_back(block->units, &unit);
        return NULL;
    }
    case AST_BLOCK: {
        /* generate new sub-block and attach a sub-scope to it */
        struct ir_unit sub_block_unit = ir_unit_block_ctr();
        struct sym_scope * sub_scope = sym_scope_new(scope);

        /* add subnodes */
        int n = darr_size(node->block.statements);
        for (int i = 0; i < n; i++) {
            struct ast_node ** subnode = darr_get(node->block.statements, i);
            ir_prog_generate_rec(*subnode,
                                 &(sub_block_unit.block),
                                 sub_scope,
                                 lineno,
                                 label);
        }

        /* now we push that new block to the program */
        darr_push_back(block->units, &sub_block_unit);
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
        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_VAR_ASSIGNMENT,
                .lineno = ++(*lineno),
                .var_asn = {
                    .dest = node->asn.sym,
                    .val = rhs,
                }
        });
        darr_push_back(block->units, &unit);
        return node->asn.sym;
    };
    case AST_DECLARATION: {
        /* Push declaration */
        struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_VAR_DECL,
                .lineno = ++(*lineno),
                .decl = { .sym = node->decl.sym }
        });
        darr_push_back(block->units, &unit);

        /* Push assignment
         * const assignment of default value if none provided
         * var assignment otherwise
         */
        if (!node->decl.rhs) {
            struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                    .type = IR_CONST_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .const_asn = {
                        .dest = sym_new(scope, NULL, node->decl.type),
                        .scalar = { .type = node->decl.type, }
                    }
                });
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
            struct ir_unit unit = ir_unit_stmt_ctr((struct ir_stmt){
                    .type = IR_VAR_ASSIGNMENT,
                    .lineno = ++(*lineno),
                    .var_asn = {
                        .dest = node->decl.sym,
                        .val = rhs,
                    }
                });
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
         * We construct CFG as we go. The entire loop thing is in one
         * seperate block because this part needs to have a seperate
         * sub-scope
         * -----------------
         * goto condition
         * -----------------
         * block:
         * ....
         * block:
         * condition: if condition goto block
         * -----------------
         */

        // create sub-block and sub-scope
        struct ir_unit subblock_unit = ir_unit_block_ctr();
        struct sym_scope * subscope = sym_scope_new(scope);

        // calculate label ids
        int block_label_id = sym_new(scope, NULL, SCAL_INTEGER)->id;
        int condition_label_id = sym_new(scope, NULL, SCAL_INTEGER)->id;

        /* add
         * goto condition:
         */
        int goto_label_indx  = ir_prog_add_goto(lineno,
                                                condition_label_id,
                                                block);

        /* add
         * block:
         */
        struct ir_unit block_label = ir_unit_stmt_ctr((struct ir_stmt){
                .type = IR_LABEL,
                .lineno = ++(*lineno),
                .label = { .id = block_label_id, },
            });
        // push block label unit to sub_block
        darr_push_back(subblock_unit.block.units, &block_label);
        int block_label_indx = darr_size(subblock_unit.block.units) - 1;

        /* add
         * while loop body
         */
        struct ast_node * body = node->while_loop.body;
        int body_n = darr_size(body->block.statements);
        for (int i = 0; i < body_n; i++) {
            struct ast_node ** child = darr_get(body->block.statements, i);
            ir_prog_generate_rec(*child,
                                 &subblock_unit.block,
                                 subscope,
                                 lineno,
                                 label);
        }

        /* add
         * condition part
         */
        int condition_label_indx = ir_prog_add_cond_goto(node->while_loop.condition,
                                                         lineno,
                                                         label,
                                                         subscope,
                                                         condition_label_id,
                                                         block_label_id,
                                                         &(subblock_unit.block));
        // now we collect the block pointers
        struct ir_unit * block_label_ref = darr_get(subblock_unit.block.units, block_label_indx);
        struct ir_unit * cjmp_ref        = darr_get(subblock_unit.block.units, condition_label_indx);

        /* The only explicit CFG edge needed here is the CJMP's taken branch
         * back to the loop body entry (block_label). All other connectivity
         * is handled by ir_wire_sequential:
         *   - outer stmt before subblock → first stmt of subblock (sequential)
         *   - all stmts within subblock wired sequentially (8→9→...→18)
         * The CJMP's not-taken (fall-through) exit has no successor,
         * which is correct: variables dead after the loop need no edge. */

        // cjmp true-branch -> block_label (loop back-edge)
        darr_push_back(cjmp_ref->succ, &block_label_ref);
        darr_push_back(block_label_ref->pred, &cjmp_ref);

        // attach the loop's sub-block to the enclosing block
        darr_push_back(block->units, &subblock_unit);

        return NULL;
    }
    };
}


struct ir_prog
ir_prog_generate(struct ast_node * node, struct sym_scope * scope)
{
    if (!node || !scope) {
        return (struct ir_prog ){
            .cnt_labels = 0,
            .cnt_lines = 0,
            .root_unit = NULL,
        };
    }
    struct ir_unit * root_unit = malloc(sizeof(struct ir_unit));
    *root_unit = ir_unit_block_ctr();
    int lineno = 0;
    int label = 0;
    ir_prog_generate_rec(node, &(root_unit->block), scope, &lineno, &label);
    return (struct ir_prog) {
        .root_unit = root_unit,
        .cnt_labels = label,
        .cnt_lines = lineno,
    };
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

static void
ir_populate_use_def(struct ir_unit * unit, int cnt_symbols)
{
    if (!unit) return;
    if (unit->type == IR_BLOCK) {
        int n = darr_size(unit->block.units);
        for (int i = 0; i < n; i++) {
            struct ir_unit * child = darr_get(unit->block.units, i);
            ir_populate_use_def(child, cnt_symbols);
        }
        return;
    }
    unit->use = bitset_ctr(cnt_symbols);
    unit->def = bitset_ctr(cnt_symbols);
    unit->out = bitset_ctr(cnt_symbols);
    unit->in  = bitset_ctr(cnt_symbols);

    switch (unit->stmt.type) {
    case IR_CONST_ASSIGNMENT:
        bitset_insert(unit->def, unit->stmt.const_asn.dest->id);
        break;
    case IR_BINOP_ASSIGNMENT:
        bitset_insert(unit->def, unit->stmt.binop_asn.dest->id);
        bitset_insert(unit->use, unit->stmt.binop_asn.val1->id);
        bitset_insert(unit->use, unit->stmt.binop_asn.val2->id);
        break;
    case IR_UNOP_ASSIGNMENT:
        bitset_insert(unit->def, unit->stmt.unop_asn.dest->id);
        bitset_insert(unit->use, unit->stmt.unop_asn.val->id);
        break;
    case IR_VAR_ASSIGNMENT:
        bitset_insert(unit->def, unit->stmt.var_asn.dest->id);
        bitset_insert(unit->use, unit->stmt.var_asn.val->id);
        break;
    case IR_VAR_DECL:
        /* Do not def the symbol here. The register will be allocated by
         * the IR_CONST_ASSIGNMENT or IR_VAR_ASSIGNMENT that immediately
         * follows the decl. Defing it here causes alloc+immediate free
         * before the assignment has a chance to use it. */
        break;
    case IR_PRINT:
        bitset_insert(unit->use, unit->stmt.print.val->id);
        break;
    case IR_LABEL:
        bitset_insert(unit->def, unit->stmt.label.id);
        break;
    case IR_CJMP:
        bitset_insert(unit->use, unit->stmt.cjmp.cond_symb->id);
        bitset_insert(unit->use, unit->stmt.cjmp.loc_label);
        break;
    case IR_JMP:
        bitset_insert(unit->use, unit->stmt.jmp.loc_label);
        break;
    }
}

/* in[u] = use[u] U (out[u] - def[u])
 * out[u] must be populated before calling.
 * Returns true if in[u] changed. */
static bool
ir_update_unit_in(struct ir_unit * unit)
{
    struct bitset * tmp = bitset_copy(unit->out);
    bitset_difference(tmp, unit->def);
    bitset_union_assign(tmp, unit->use);

    bool changed = !bitset_equal(tmp, unit->in);
    bitset_assign(unit->in, tmp);
    bitset_destroy(tmp);
    return changed;
}

/* out[u] U= in[s] for each successor s.
 * then recompute in[u]
 * recurse into IR_BLOCK units (which carry no liveness sets themselves) */
static bool
ir_update_in_out(struct ir_unit * unit)
{
    if (!unit) return false;

    if (unit->type == IR_BLOCK) {
        bool changed = false;
        int n = darr_size(unit->block.units);
        /* Traverse in reverse: liveness is a backward dataflow problem.
         * A statement's out = union of its successors' in. The sequential
         * successor of child[i] is child[i+1], so we must process child[i+1]
         * first so its in is up-to-date when child[i] reads it. */
        for (int i = n - 1; i >= 0; i--) {
            struct ir_unit * child = darr_get(unit->block.units, i);
            changed = ir_update_in_out(child) || changed;
        }
        return changed;
    }

    /* recompute out from successors in's */
    int n_succ = darr_size(unit->succ);
    for (int s = 0; s < n_succ; s++) {
        struct ir_unit ** succ = darr_get(unit->succ, s);
        bitset_union_assign(unit->out, (*succ)->in);
    }

    printf("%2d. succ_size: %3d\n",
           unit->stmt.lineno,
           darr_size(unit->succ));

    return ir_update_unit_in(unit);
}

void
ir_cfg_analysis(struct ir_prog * prog, struct sym_scope * scope)
{
    if (!prog) return;
    int cnt_sym = sym_scope_cnt_symbols(scope);
    ir_populate_use_def(prog->root_unit, cnt_sym);
    ir_wire_sequential(prog->root_unit);   // ← add this
    while (ir_update_in_out(prog->root_unit))
        ;
}


/* Wire sequential fall-through edges within a block.
 * For each consecutive pair of stmt units, prev->succ includes next
 * and next->pred includes prev. Recurse into sub-blocks first so
 * inner units are fully linked before we look at the outer block. */
static void
ir_wire_sequential(struct ir_unit * unit)
{
    if (!unit) return;
    if (unit->type == IR_STMT) return;

    int n = darr_size(unit->block.units);

    /* First recurse so inner blocks are wired */
    for (int i = 0; i < n; i++) {
        struct ir_unit * child = darr_get(unit->block.units, i);
        ir_wire_sequential(child);
    }

    /* Then wire consecutive children of this block.
     * "last stmt" of child i -> "first stmt" of child i+1 */
    for (int i = 0; i + 1 < n; i++) {
        struct ir_unit * a = darr_get(unit->block.units, i);
        struct ir_unit * b = darr_get(unit->block.units, i + 1);

        /* Get the last stmt unit reachable from a */
        struct ir_unit * last_of_a = a;
        while (last_of_a->type == IR_BLOCK) {
            int m = darr_size(last_of_a->block.units);
            if (m == 0) break;
            last_of_a = darr_get(last_of_a->block.units, m - 1);
        }

        /* Get the first stmt unit reachable from b */
        struct ir_unit * first_of_b = b;
        while (first_of_b->type == IR_BLOCK) {
            int m = darr_size(first_of_b->block.units);
            if (m == 0) break;
            first_of_b = darr_get(first_of_b->block.units, 0);
        }

        if (last_of_a->type == IR_STMT && first_of_b->type == IR_STMT) {
            darr_push_back(last_of_a->succ, &first_of_b);
            darr_push_back(first_of_b->pred, &last_of_a);
        }
    }
}
