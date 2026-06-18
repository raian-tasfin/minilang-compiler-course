#include "cg.h"
#include "../vm/vm-core/vm-core.h"
#include "../vm/vm-core/vm-definitions.h"
#include <locale.h>


/**************
 * Structures *
 **************/
struct cg_ctx {
    struct darr * reg_of_sym; // int. register. int. (-1 means unassigned)

    /* Label line does not produce an instruction. Instead we record
     * the  index of the instruction immediately produced by the line
     * after the label in the ir form.
     */
    struct darr * label_pos;  // int. position of label

    /* Before a jump we generate a const to register move instruction
     * to store the jump location. Such a mov instruction takes two
     * units, one for the instruction and one for the raw value.
     *
     * In the first pass we generate
     *     mov reg label_id
     *     jmp reg / cjmp cond_reg reg
     * In the jump pos array we store the location of the label_id
     * value of the produced mov instruction.
     *
     * In the second pass we read raw values from the indices of
     * jump_pos. Those values are the indices of labels. We replace
     * those with the actual line numbers pointed to by the labels.
     */
    struct darr * jump_pos;   // int. position of jump statements

    int cnt_sym;

    int sym_at_reg[VM_REGISTER_CNT];
    struct ir_prog * ir_prog;
};



/************************
 * Forward Declarations *
 ************************/
bool cg_generate_code_rec(struct cg_ctx * ctx, struct ir_unit * unit, struct darr * program);

union vm_instr_view cg_generate_const_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_var_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_binop_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_unop_asn(struct cg_ctx * ctx, struct ir_stmt stmt);
union vm_instr_view cg_generate_prnt_asn( struct cg_ctx * ctx, struct ir_stmt stmt);


bool cg_ir_register_alloc_pass(struct cg_ctx * ctx, struct ir_unit unit);
void cg_ir_register_free_pass(struct cg_ctx * ctx, struct ir_unit unit);

int cg_scalar_to_int(struct ir_scalar scalar);

enum vm_op cg_ir_to_vm_binop(enum ir_binop op);
enum vm_op cg_ir_to_vm_unop(enum ir_unop op);

/******************
 * Implmentations *
 ******************/
struct cg_ctx *
cg_ctx_init(struct ir_prog * ir_prog, int cnt_sym)
{
    struct cg_ctx * ctx = NULL;
    struct darr * luse = NULL;
    int none = -1;

    // Ensure valid program
    if (!ir_prog || !ir_prog->root_unit) return NULL;

    // Allocate context
    if (!(ctx = malloc(sizeof(struct cg_ctx)))) goto error;

    // initialize context
    if (!(ctx->reg_of_sym = darr_init(sizeof(int)))) goto error;
    darr_ensure_index(ctx->reg_of_sym, cnt_sym, &none);

    if (!(ctx->label_pos = darr_init(sizeof(int)))) goto error;
    darr_ensure_index(ctx->label_pos, cnt_sym, &none);

    if (!(ctx->jump_pos = darr_init(sizeof(int)))) goto error;

    ctx->cnt_sym = cnt_sym;

    for (int r = 0; r < VM_REGISTER_CNT; r++) ctx->sym_at_reg[r] = none;
    ctx->ir_prog = ir_prog;

    return ctx;

error:
    cg_ctx_destroy(&ctx);
    return NULL;
}


struct darr *
cg_generate_code(struct cg_ctx * ctx)
{
    struct darr * program = NULL;

    if (!ctx) goto error;
    if (!(program = darr_init(sizeof(union vm_instr_view)))) goto error;
    if (!cg_generate_code_rec(ctx, ctx->ir_prog->root_unit, program)) goto error;

    /* Second pass: each entry in jump_pos is the program index of a raw
     * label-id placeholder. The placeholder value is the symbol ID of the
     * label, which indexes into label_pos to get the resolved instruction
     * address. Patch every placeholder in-place. */
    for (int i = 0; i < darr_size(ctx->jump_pos); i++) {
        int placeholder_idx = *(int*)darr_get(ctx->jump_pos, i);
        int label_sym_id    = ((union vm_instr_view*)darr_get(program, placeholder_idx))->raw;
        int target_pc       = *(int*)darr_get(ctx->label_pos, label_sym_id);
        union vm_instr_view patched = { .raw = target_pc };
        darr_set(program, placeholder_idx, &patched);
    }

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
    darr_destroy(&(*ctx)->reg_of_sym);
    darr_destroy(&(*ctx)->label_pos);
    darr_destroy(&(*ctx)->jump_pos);
}


bool
cg_generate_code_rec(struct cg_ctx * ctx, struct ir_unit * unit, struct darr * program)
{
    if (!ctx) return false;
    if (!program) return false;

    if (unit->type == IR_BLOCK) {
        int n = darr_size(unit->block.units);
        for (int i = 0; i < n; i++) {
            struct ir_unit * subunit = darr_get(unit->block.units, i);
            if (!cg_generate_code_rec(ctx, subunit, program)) return false;
        }
        return true;
    }

    if (!cg_ir_register_alloc_pass(ctx, *unit)) return false;

    switch (unit->stmt.type) {
    case IR_CONST_ASSIGNMENT: {
        union vm_instr_view instr_view = cg_generate_const_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &instr_view)) return false;

        union vm_instr_view const_view = { .raw = cg_scalar_to_int(unit->stmt.const_asn.scalar) };
        if (!darr_push_back(program, &const_view)) return false;
        break;
    }
    case IR_BINOP_ASSIGNMENT: {
        union vm_instr_view view = cg_generate_binop_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &view)) return false;
        break;
    }
    case IR_UNOP_ASSIGNMENT: {
        union vm_instr_view view = cg_generate_unop_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &view)) return false;
        break;
    }
    case IR_PRINT: {
        union vm_instr_view view = cg_generate_prnt_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &view)) return false;
        break;
    }
    case IR_VAR_ASSIGNMENT: {
        union vm_instr_view instr_view = cg_generate_var_asn(ctx, unit->stmt);
        if (!darr_push_back(program, &instr_view)) return false;
        break;
    }
    case IR_VAR_DECL: {
        /* Skip for now.
         * For now we are allocating one register per symbol. Since we
         * don't have arrays, we are unlikely to require additional
         * memory at all. But in case we do, this statement will have
         * to assign memory offset to the symbol.
         */
        break;
    }
    case IR_LABEL: {
        int next_indx = darr_size(program);
        if (!darr_set(ctx->label_pos, unit->stmt.label.id, &next_indx)) return false;
        break;
    }
    case IR_JMP: {
        int label_id = unit->stmt.jmp.loc_label;
        int reg = *(int*)darr_get(ctx->reg_of_sym, label_id);

        // add move
        union vm_instr_view mov_view = {
            .mov = {
                .op = VM_MOV,
                .dest = reg,
                .flag = VM_MOV_CONST_TO_REG,
            }
        };
        if (!darr_push_back(program, &mov_view)) return false;

        // add raw value. label id for now
        union vm_instr_view label_val = { .raw = label_id, };
        if (!darr_push_back(program, &label_val)) return false;

        // record raw values index
        int indx = darr_size(program) - 1;
        if (!darr_push_back(ctx->jump_pos, &indx)) return false;

        // add jump
        union vm_instr_view jmp = {
            .jmp = {
                .op = VM_JMP,
                .loc_reg = reg,
            },
        };
        if (!darr_push_back(program, &jmp)) return false;
        break;
    }
    case IR_CJMP: {
        /* Handle jump label part. similar to jump */
        int label_id = unit->stmt.cjmp.loc_label;
        int reg = *(int*)darr_get(ctx->reg_of_sym, label_id);
        union vm_instr_view mov_view = {
            .mov = {
                .op = VM_MOV,
                .dest = reg,
                .flag = VM_MOV_CONST_TO_REG,
            }
        };
        if (!darr_push_back(program, &mov_view)) return false;
        union vm_instr_view label_val = { .raw = label_id, };
        if (!darr_push_back(program, &label_val)) return false;
        int cjmp_indx = darr_size(program) - 1;
        if (!darr_push_back(ctx->jump_pos, &cjmp_indx)) return false;

        /* add cjmp */
        union vm_instr_view cjmp = {
            .cjmp = {
                .op = VM_JMP,
                .loc_reg = reg,
                .cond_reg = *(int*)darr_get(ctx->reg_of_sym, unit->stmt.cjmp.cond_symb->id),
            },
        };
        if (!darr_push_back(program, &cjmp)) return false;
        break;
    }
    }
    return true;
}

union vm_instr_view
cg_generate_const_asn(struct cg_ctx * ctx, struct ir_stmt stmt)
{
    union vm_instr_view view = {
        .mov = {
            .dest = *(int*)darr_get(ctx->reg_of_sym, stmt.const_asn.dest->id),
            .op   = VM_MOV,
            .flag = VM_MOV_CONST_TO_REG,
        },
    };
    return view;
}




union vm_instr_view
cg_generate_var_asn(struct cg_ctx * ctx, struct ir_stmt stmt)
{
    union vm_instr_view view = {
        .mov = {
            .dest = *(int*)darr_get(ctx->reg_of_sym, stmt.var_asn.dest->id),
            .src  = *(int*)darr_get(ctx->reg_of_sym, stmt.var_asn.val->id),
            .op   = VM_MOV,
            .flag = VM_MOV_REG_TO_REG,
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
            .dest = *(int*)darr_get(ctx->reg_of_sym, stmt.binop_asn.dest->id),
            .arg1 = *(int*)darr_get(ctx->reg_of_sym, stmt.binop_asn.val1->id),
            .arg2 = *(int*)darr_get(ctx->reg_of_sym, stmt.binop_asn.val2->id),
        }
    };
    return view;;
}

union vm_instr_view
cg_generate_unop_asn(struct cg_ctx * ctx, struct ir_stmt stmt)
{
    union vm_instr_view view = {
        .un = {
            .op   = cg_ir_to_vm_unop(stmt.unop_asn.op),
            .dest = *(int*)darr_get(ctx->reg_of_sym, stmt.unop_asn.dest->id),
            .arg = *(int*)darr_get(ctx->reg_of_sym, stmt.unop_asn.val->id),
        }
    };
    return view;;
}

union vm_instr_view
cg_generate_prnt_asn( struct cg_ctx * ctx, struct ir_stmt stmt)
{
    uint16_t flags = 0;
    if (stmt.print.val->type == SCAL_BOOLEAN) flags |= VM_PRNT_BOOLEAN;

    union vm_instr_view view = {
        .print = {
            .op = VM_PRNT,
            .flags = flags,
            .reg = *(int*)darr_get(ctx->reg_of_sym, stmt.print.val->id),
        }
    };
    return view;
}


int
cg_scalar_to_int(struct ir_scalar scalar)
{
    switch (scalar.type) {
    case SCAL_BOOLEAN: return scalar.boolean ? 1 : 0;
    case SCAL_INTEGER: return scalar.integer;
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
    case IR_LT: return VM_LT;
    case IR_LE: return VM_LE;
    case IR_GT: return VM_GT;
    case IR_GE: return VM_GE;
    case IR_NE: return VM_NE;
    case IR_EQ: return VM_EQ;
    }
}


enum vm_op
cg_ir_to_vm_unop(enum ir_unop op)
{
    switch (op) {
    case IR_NEG: return VM_NEG;
    case IR_NOT: return VM_NOT;
    }
}


bool
cg_ir_register_alloc_pass(struct cg_ctx * ctx, struct ir_unit unit)
{
    int none = -1;
    for (int sym = 0; sym < ctx->cnt_sym; sym++) {
        if (!bitset_contains(unit.def, sym)) continue;
        bool found = false;
        for (int r = 0; r < VM_REGISTER_CNT; r++) {
            if (ctx->sym_at_reg[r] != none) continue;
            ctx->sym_at_reg[r] = sym;
            if (!darr_set(ctx->reg_of_sym, sym, &r)) return false;
            found = true;
            break;
        }
        if (!found) return false;
    }
    return true;
}

void
cg_ir_register_free_pass(struct cg_ctx * ctx, struct ir_unit unit)
{
    int none = -1;
    for (int sym = 0; sym < ctx->cnt_sym; sym++) {
        if (!bitset_contains(unit.out, sym)) {
            int r = *(int*)darr_get(ctx->reg_of_sym, sym);
            darr_set(ctx->reg_of_sym, sym, &none);
            ctx->sym_at_reg[r] = none;
        }
    }
}
