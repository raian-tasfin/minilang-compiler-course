#include "cg.h"
#include "../darr/darr.h"
#include "../vm/vm-core/vm-core.h"
#include "../intrep/interp.h"
#include <assert.h>
#include <setjmp.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>


/**************
 * Structures *
 **************/
struct cg_ctx {
    struct cg_darr * last_use;        // line number. int
    struct cg_darr * reg_of_sym;      // register int. (need -1)
    int sym_at_reg[VM_REGISTER_CNT];
    struct ir_block * block;
};


/*********************
 * Private Utilities *
 *********************/
enum vm_op
cg_ir_to_vm_op(enum ir_type type)
{
    switch (type) {
    case IR_ADD: return VM_ADD;
    case IR_SUB: return VM_SUB;
    case IR_MUL: return VM_MUL;
    case IR_MOD: return VM_MOD;
    case IR_DIV: return VM_DIV;
    }
}

static struct cg_darr *
cg_get_last_use(struct ir_block * block)
{
    struct cg_darr * darr = NULL;

    int none = -1;

    if (!block) {
        fprintf(stderr, "[CG Last Use]: Empty block provided. No last use calculated.\n");
        goto error;
    }
    if (!(darr = cg_darr_init(sizeof(int)))) {
        fprintf(stderr, "[CG Last Use]: Error initializing last use array.\n");
        goto error;
    }

    for (int lineno = 0; lineno < block->size; lineno++) {
        struct ir_stmt stmt = block->stmts[lineno];
        switch(stmt.type) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_MOD:
            if (stmt.arg1.type == IR_SYM) {
                if (!cg_darr_ensure_index(darr, stmt.arg1.sym->id, &none)) {
                    fprintf(stderr,
                            "[CG Last Use]: Error allocating symbol index %d for arg1 in line %d\n",
                            stmt.arg1.sym->id,
                            lineno);
                    goto error;
                }
                if (!cg_darr_set(darr, stmt.arg1.sym->id, &lineno)) {
                    fprintf(stderr,
                            "[CG Last Use]: Error setting last use of index %d for arg1 to line %d\n",
                            stmt.arg1.sym->id,
                            lineno);
                    goto error;
                }
            }
            if (stmt.arg2.type == IR_SYM) {
                if (!cg_darr_ensure_index(darr, stmt.arg2.sym->id, &none)) {
                    fprintf(stderr,
                            "[CG Last Use]: Error allocating symbol index %d for arg2 in line %d\n",
                            stmt.arg2.sym->id,
                            lineno);
                    goto error;
                }
                if (!cg_darr_set(darr, stmt.arg2.sym->id, &lineno)) {
                    fprintf(stderr,
                            "[CG Last Use]: Error setting last use of index %d for arg2 to line %d\n",
                            stmt.arg2.sym->id,
                            lineno);
                    goto error;
                }
            }
            break;
        }

    }
    return darr;

error:
    cg_darr_destroy(&darr);
    return NULL;
}



/*******************
 * Code Generators *
 *******************/
/* Ensure that symbol of id `id' has a register at line `lineno'.
 */
int
cg_new_register(struct cg_ctx * ctx, int lineno)
{
    if (!ctx) return -1;
    for (int r = 0; r < VM_REGISTER_CNT; r++) {
        if (ctx->sym_at_reg[r] == -1) return r;
        if (*(int*)cg_darr_get(ctx->last_use, r) < lineno) return r;
    }
    return -1;
}

int
cg_ensure_register(struct cg_ctx * ctx, int symid, int lineno)
{
    if (lineno == -1) return -1;
    assert(ctx);
    assert(0 <= symid);
    assert(symid < cg_darr_size(ctx->last_use));

    /* Check for existing register allocation */
    if (*(int*)cg_darr_get(ctx->reg_of_sym, symid) != -1) return *(int*)cg_darr_get(ctx->reg_of_sym, symid);

    /* Look for new register */
    for (int r = 0; r < VM_REGISTER_CNT; r++) {
        /* Assign free register */
        if (ctx->sym_at_reg[r] == -1) {
            cg_darr_set(ctx->reg_of_sym, symid, &r);
            ctx->sym_at_reg[r] = symid;
            return r;
        }

        /* Otherwise check for stale register */
        int sym_at_reg = ctx->sym_at_reg[r];
        int last_use = *(int*)cg_darr_get(ctx->last_use, sym_at_reg);
        if (last_use < lineno) {
            cg_darr_set(ctx->reg_of_sym, symid, &r);
            ctx->sym_at_reg[r] = symid;
            return r;
        }
    }
    return -1;
}

bool
cg_generate_mov_rc(struct cg_ctx * ctx, struct cg_darr * program, int dest, int val)
{
    if (!ctx) return false;
    if (!program) return false;
    union vm_instr_view mov_part = {
        .mov = {
            .dest = dest,
            .flag = VM_MOV_CONST_TO_REG,
            .op = VM_MOV
        }
    };
    union vm_instr_view val_part = { .raw = val };

    if (!cg_darr_push_back(program, &mov_part)) return false;
    if (!cg_darr_push_back(program, &val)) return false;
    return true;
}



int
cg_ensure_arg_register(struct cg_ctx * ctx, struct cg_darr * program, struct ir_arg arg, int lineno)
{
    /* If symbol, just allocate a register */
    if (arg.type == IR_SYM) return cg_ensure_register(ctx, arg.sym->id, lineno);

    /* For constants we need to generate a mov instruction */
    int new_register = cg_new_register(ctx, lineno);
    if (new_register == -1) return -1;

    if (!cg_generate_mov_rc(ctx, program, new_register, arg.INTEGER)) return -1;
    return new_register;
}



bool
cg_generate_binop(struct cg_ctx * ctx, struct cg_darr * program, struct ir_stmt stmt, int lineno)
{
    int arg1_reg = cg_ensure_arg_register(ctx, program, stmt.arg1, lineno);
    if (arg1_reg == -1) return false;

    int arg2_reg = cg_ensure_arg_register(ctx, program, stmt.arg2, lineno);
    if (arg2_reg == -1) return false;

    int dest_reg = cg_ensure_register(ctx, stmt.dst->id, lineno);
    if (dest_reg == -1) return false;

    union vm_instr_view view = {
        .bin = {
            .op = cg_ir_to_vm_op(stmt.type),
            .arg1 = arg1_reg,
            .arg2 = arg2_reg,
            .dest = dest_reg,
        }
    };
    if (!cg_darr_push_back(program, &view)) return false;
    return true;
}

/**************
 * Public API *
 **************/
void
cg_ctx_destroy(struct cg_ctx ** ctx)
{
    if (!ctx || !*ctx) return;
    cg_darr_destroy(&ctx[0]->last_use);
    cg_darr_destroy(&ctx[0]->reg_of_sym);
    free(*ctx);
    *ctx = NULL;
}


struct cg_ctx *
cg_ctx_init(struct ir_block * block)
{
    struct cg_ctx * ctx = NULL;
    int none = -1;

    /* Ensure block */
    if (!block) return NULL;

    /* Allocate context */
    if (!(ctx = malloc(sizeof(struct cg_ctx)))) goto error;

    /* Get last use */
    if (!(ctx->last_use = cg_get_last_use(block))) goto error;

    /* Allocate reg_of_sym */
    if (!(ctx->reg_of_sym = cg_darr_init(sizeof(int)))) goto error;
    if (!(cg_darr_resize(ctx->reg_of_sym, cg_darr_size(ctx->last_use), &none))) goto error;

    /* Fill sym_at_reg */
    for (int i = 0; i < VM_REGISTER_CNT; i++) {
        ctx->sym_at_reg[i] = -1;
    }

    /* Add block to context */
    ctx->block = block;

    return ctx;
error:
    cg_ctx_destroy(&ctx);
    return NULL;
}


// cg_darr of type vm_instr_view
struct cg_darr *
cg_generate_code(struct cg_ctx * ctx)
{
    struct cg_darr * program = NULL;
    int arg1_reg;
    int arg2_reg;
    int dest_reg;

    /* Ensure context */
    if (!ctx) goto error;

    /* Init program array */
    if (!(program = cg_darr_init(sizeof(union vm_instr_view)))) goto error;

    for (int lineno = 0; lineno < ctx->block->size; lineno++) {
        struct ir_stmt stmt = ctx->block->stmts[lineno];
        switch(stmt.type) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_MOD:
            if (!cg_generate_binop(ctx, program, stmt, lineno)) goto error;
            break;
        }

    }
    return program;

error:
    cg_darr_destroy(&program);
    return NULL;
}
