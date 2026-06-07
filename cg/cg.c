#include "cg.h"
#include "../vm/vm-core/vm-core.h"
#include "../vm/vm-core/vm-definitions.h"
#include <errno.h>


/**************
 * Structures *
 **************/
struct cg_ctx {
    struct darr *    last_use;   // line number. int.
    struct darr *    reg_of_sym; // register. int. (-1 means unassigned)
    int              sym_at_reg[VM_REGISTER_CNT];
    struct ir_unit * root_unit;
};



/************************
 * Forward Declarations *
 ************************/
static struct darr *
cg_get_last_use(struct ir_unit * unit);

bool cg_get_last_use_rec(struct darr * luse, struct ir_unit * unit);
bool cg_update_last_use(struct darr * luse, int id, int lineno, int none);
bool cg_generate_code_rec(struct cg_ctx * ctx, struct ir_unit * unit, struct darr * program);
int  cg_alloc_register_sym(struct cg_ctx * ctx, int symid, int lineno);

union vm_instr_view cg_generate_const_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_binop_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_prnt_asn( struct cg_ctx * ctx, struct ir_stmt stmt);

int cg_scalar_to_int(struct ir_scalar scalar);

enum vm_op cg_ir_to_vm_binop(enum ir_binop op);

/******************
 * Implmentations *
 ******************/
struct cg_ctx *
cg_ctx_init(struct ir_unit * root_unit)
{
    struct cg_ctx * ctx = NULL;
    struct darr * luse = NULL;
    int none = -1;

    // Ensure root ir unit
    if (!root_unit) return NULL;

    // Allocate context
    if (!(ctx = malloc(sizeof(struct cg_ctx)))) goto error;

    // get last use
    if (!(ctx->last_use = cg_get_last_use(root_unit))) goto error;

    // allocate reg_of_sym
    if (!(ctx->reg_of_sym = darr_init(sizeof(int)))) goto error;
    if (!(darr_resize(ctx->reg_of_sym, darr_size(ctx->last_use), &none))) goto error;

    // fill sym_at_reg
    for (int i = 0; i < VM_REGISTER_CNT; i++) ctx->sym_at_reg[i] = -1;

    // add root unit to context
    ctx->root_unit = root_unit;

    return ctx;

error:
    cg_ctx_destroy(&ctx);
    darr_destroy(&luse);
    return NULL;
}


struct darr *
cg_generate_code(struct cg_ctx * ctx)
{
    struct darr * program = NULL;

    if (!ctx) goto error;
    if (!(program = darr_init(sizeof(union vm_instr_view)))) goto error;
    if (!cg_generate_code_rec(ctx, ctx->root_unit, program)) goto error;

    union vm_instr_view exit_view = {
        .base = {
            .op = VM_EXIT
        }
    };

    if (!(darr_push_back(program, &exit_view))) goto error;

    return program;

error:
    darr_destroy(&program);
    return NULL;
}


void
cg_ctx_destroy(struct cg_ctx ** ctx)
{
    if (!ctx || !*ctx) return;
    darr_destroy(&(*ctx)->last_use);
    darr_destroy(&(*ctx)->reg_of_sym);
}


static struct darr *
cg_get_last_use(struct ir_unit * unit)
{
    struct darr * luse = NULL;

    if (!unit) goto error;
    if (!(luse = darr_init(sizeof(int)))) goto error;
    if (!cg_get_last_use_rec(luse, unit)) goto error;
    return luse;

 error:
    darr_destroy(&luse);
    return NULL;
}


bool
cg_get_last_use_rec(struct darr * luse, struct ir_unit * unit)
{
    int none = -1;

    if (!luse) return false;
    if (!unit) return false;
    if (unit->type == IR_BLOCK) {
        int n = darr_size(unit->block);
        for (int i = 0; i < n; i++) {
            struct ir_unit * subunit= darr_get(unit->block, i);
            if (!cg_get_last_use_rec(luse, subunit)) return false;
        }
        return true;
    }

    switch (unit->stmt.type) {
    case IR_CONST_ASSIGNMENT: /* no read */ return true;
    case IR_BINOP_ASSIGNMENT: {
        if (!(cg_update_last_use(luse, unit->stmt.binop_asn.val1->id, unit->stmt.lineno, none))) return false;
        if (!(cg_update_last_use(luse, unit->stmt.binop_asn.val2->id, unit->stmt.lineno, none))) return false;
        return true;
    }
    case IR_PRINT:
        if (!(cg_update_last_use(luse, unit->stmt.print.val->id, unit->stmt.lineno, none))) return false;
        return true;
    }
}


bool
cg_update_last_use(struct darr * luse, int id, int lineno, int none)
{
    if (!darr_ensure_index(luse, id, &none)) return false;
    if (!darr_set(luse, id, &lineno)) return false;
    return true;
}



bool
cg_generate_code_rec(struct cg_ctx * ctx, struct ir_unit * unit, struct darr * program)
{
    if (!ctx) return false;
    if (!program) return false;

    if (unit->type == IR_BLOCK) {
        int n = darr_size(unit->block);
        for (int i = 0; i < n; i++) {
            struct ir_unit * subunit = darr_get(unit->block, i);
            if (!cg_generate_code_rec(ctx, subunit, program)) return false;
        }
        return true;
    }

    switch (unit->stmt.type) {
    case IR_CONST_ASSIGNMENT: {
        union vm_instr_view instr_view = cg_generate_const_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &instr_view)) return false;

        union vm_instr_view const_view = { .raw = cg_scalar_to_int(unit->stmt.const_asn.scalar) };
        if (!darr_push_back(program, &const_view)) return false;
        return true;
    }
    case IR_BINOP_ASSIGNMENT: {
        union vm_instr_view view = cg_generate_binop_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &view)) return false;
        return true;
    }
    case IR_PRINT: {
        union vm_instr_view view = cg_generate_prnt_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &view)) return false;
        return true;
    }
    }
}


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


union vm_instr_view
cg_generate_const_asn(struct cg_ctx * ctx, struct ir_stmt stmt)
{
    union vm_instr_view view = {
        .mov = {
            .dest = cg_alloc_register_sym(ctx, stmt.const_asn.dest->id, stmt.lineno),
            .flag = VM_MOV_CONST_TO_REG,
            .op   = VM_MOV,
        }
    };
    return view;
}

union vm_instr_view
cg_generate_binop_asn(struct cg_ctx * ctx, struct ir_stmt stmt)
{
    union vm_instr_view view = {
        .bin = {
            .op   = cg_ir_to_vm_binop(stmt.binop_asn.op),
            .dest = cg_alloc_register_sym(ctx, stmt.binop_asn.dest->id, stmt.lineno),
            .arg1 = cg_alloc_register_sym(ctx, stmt.binop_asn.val1->id, stmt.lineno),
            .arg2 = cg_alloc_register_sym(ctx, stmt.binop_asn.val2->id, stmt.lineno),
        }
    };
    return view;;
}

union vm_instr_view
cg_generate_prnt_asn( struct cg_ctx * ctx, struct ir_stmt stmt)
{
    uint16_t flags = 0;
    if (stmt.print.val->type == IR_BOOLEAN) flags |= VM_PRNT_BOOLEAN;

    union vm_instr_view view = {
        .print = {
            .op = VM_PRNT,
            .flags = flags,
            .reg = cg_alloc_register_sym(ctx, stmt.print.val->id, stmt.lineno)
        }
    };
    return view;
}


int
cg_scalar_to_int(struct ir_scalar scalar)
{
    switch (scalar.type) {
    case IR_BOOLEAN: return scalar.boolean ? 1 : 0;
    case IR_INTEGER: return scalar.integer;
    }
}


enum vm_op
cg_ir_to_vm_binop(enum ir_binop op)
{
    switch (op) {
    case IR_ADD: return VM_ADD;
    case IR_SUB: return VM_SUB;
    case IR_MUL: return VM_MUL;
    case IR_DIV: return VM_DIV;
    case IR_MOD: return VM_MOD;
    case IR_AND: return VM_AND;
    case IR_OR : return VM_OR;
    case IR_XOR: return VM_XOR;
    }
}
