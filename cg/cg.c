#include "cg.h"
#include "../darr/darr.h"
#include "../vm/vm-core/vm-core.h"
#include "../vm/vm-core/vm-definitions.h"
#include "../intrep/interp.h"
#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>


/**************
 * Structures *
 **************/
struct cg_ctx {
    struct darr * last_use;        // line number. int
    struct darr * reg_of_sym;      // register int. (need -1)
    int sym_at_reg[VM_REGISTER_CNT];
    struct ir_block * block;
};


/*********************
 * Private Utilities *
 *********************/
enum vm_op
cg_ir_to_vm_op(enum ir_binop op)
{
    switch (op) {
    case IR_ADD: return VM_ADD;
    case IR_SUB: return VM_SUB;
    case IR_MUL: return VM_MUL;
    case IR_MOD: return VM_MOD;
    case IR_DIV: return VM_DIV;
    }
}

/* bool */
/* cg_generate_const_assignment(struct ir_block * block, ) */
bool
cg_update_last_use(struct darr * program, int id, int lineno, int none)
{
    if (!darr_ensure_index(program, id, &none)) return false;
    if (!darr_set(program, id, &lineno)) return false;
    return true;
}


static struct darr *
cg_get_last_use(struct ir_block * block)
{
    struct darr * luse = NULL;
    int none = -1;
    if (!block) {
        fprintf(stderr, "[CG Last Use]: Empty block provided. No last use calculated.\n");
        goto error;
    }
    if (!(luse = darr_init(sizeof(int)))) {
        fprintf(stderr, "[CG Last Use]: Error initializing last use array.\n");
        goto error;
    }

    for (int lineno = 0; lineno < block->size; lineno++) {
        struct ir_stmt stmt = block->stmts[lineno];
        switch(stmt.type) {
        case IR_CONST_ASSIGNMENT: // no read
            break;
        case IR_BINOP_ASSIGNMENT: {
            if (!cg_update_last_use(luse, stmt.binop_asn.arg1.sym->id, lineno, none)) goto error;
            if (!cg_update_last_use(luse, stmt.binop_asn.arg2.sym->id, lineno, none)) goto error;
            break;
        };
        case IR_PRINT: {
            if (!cg_update_last_use(luse, stmt.prnt.arg.sym->id, lineno, none)) goto error;
            break;
        }
        }
    }
    return luse;

error:
    darr_destroy(&luse);
    return NULL;
}

/****************************
 * Register Context Manager *
 ****************************/
int
cg_alloc_register_sym(struct cg_ctx * ctx, int symid, int lineno)
{
    int none = -1;
    if (!ctx || symid < 0) return none;

    if (!darr_ensure_index(ctx->reg_of_sym, symid, &none)) return none;
    if (!darr_ensure_index(ctx->last_use, symid, &none)) return none;

    int reg_of_sym = *(int*)darr_get(ctx->reg_of_sym, symid);
    if (reg_of_sym != -1) return reg_of_sym;

    for (int r = 0; r < VM_REGISTER_CNT; r++) {
        int current_sym = ctx->sym_at_reg[r];
        /* Free Slot */
        if (current_sym == -1) {
            ctx->sym_at_reg[r] = symid;
            darr_set(ctx->reg_of_sym, symid, &r);
            return r;
        }
        /* Stale Register Check */
        int last_use_line = *(int*)darr_get(ctx->last_use, current_sym);
        if (last_use_line < lineno) {
            darr_set(ctx->reg_of_sym, current_sym, &none);
            ctx->sym_at_reg[r] = symid;
            darr_set(ctx->reg_of_sym, symid, &r);
            return r;
        }
    }
    return none;
}


/*******************
 * Code Generators *
 *******************/
static union vm_instr_view
cg_generate_prnt(struct cg_ctx * ctx, struct ir_stmt_prnt stmt, int lineno)
{
    union vm_instr_view view = {
        .print = {
            .op = VM_PRNT,
            .reg = cg_alloc_register_sym(ctx, stmt.arg.sym->id, lineno)
        }
    };
    return view;
}

static union vm_instr_view
cg_generate_const_asn(struct cg_ctx * ctx, struct ir_stmt_const_assignment stmt, int lineno)
{
    union vm_instr_view view = {
        .mov = {
            .dest = cg_alloc_register_sym(ctx, stmt.dest->id, lineno),
            .flag = VM_MOV_CONST_TO_REG,
            .op = VM_MOV,
        }
    };
    return view;
}


static union vm_instr_view
cg_generate_binop_asn(struct cg_ctx * ctx, struct ir_stmt_binop_assignment stmt, int lineno)
{
    union vm_instr_view view = {
        .bin = {
            .op = cg_ir_to_vm_op(stmt.op),
            .dest = cg_alloc_register_sym(ctx, stmt.dst->id, lineno),
            .arg1 = cg_alloc_register_sym(ctx, stmt.arg1.sym->id, lineno),
            .arg2 = cg_alloc_register_sym(ctx, stmt.arg2.sym->id, lineno),
        }
    };
    return view;
}


/**************
 * Public API *
 **************/
void
cg_ctx_destroy(struct cg_ctx ** ctx)
{
    if (!ctx || !*ctx) return;
    darr_destroy(&ctx[0]->last_use);
    darr_destroy(&ctx[0]->reg_of_sym);
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
    if (!(ctx->reg_of_sym = darr_init(sizeof(int)))) goto error;
    if (!(darr_resize(ctx->reg_of_sym, darr_size(ctx->last_use), &none))) goto error;

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


// darr of type vm_instr_view
struct darr *
cg_generate_code(struct cg_ctx * ctx)
{
    struct darr * program = NULL;

    /* Ensure context */
    if (!ctx) goto error;

    /* Init program array */
    if (!(program = darr_init(sizeof(union vm_instr_view)))) goto error;

    for (int lineno = 0; lineno < ctx->block->size; lineno++) {
        struct ir_stmt stmt = ctx->block->stmts[lineno];
        switch(stmt.type) {
        case IR_PRINT: {
            union vm_instr_view view = cg_generate_prnt(ctx, stmt.prnt, lineno);
            if (!darr_push_back(program, &view)) goto error;
            break;
        }
        case IR_CONST_ASSIGNMENT: {
            union vm_instr_view view = cg_generate_const_asn(ctx, stmt.const_asn, lineno);
            if (!darr_push_back(program, &view)) goto error;
            view.raw = stmt.const_asn.val;
            if (!darr_push_back(program, &view)) goto error;
            break;
        }
        case IR_BINOP_ASSIGNMENT: {
            union vm_instr_view view = cg_generate_binop_asn(ctx, stmt.binop_asn, lineno);
            if (!darr_push_back(program, &view)) goto error;
            break;
        }
        default:
            break;
        }
    }
    union vm_instr_view view = { .base.op = VM_EXIT };
    darr_push_back(program, &view);
    return program;

error:
    darr_destroy(&program);
    return NULL;
}
