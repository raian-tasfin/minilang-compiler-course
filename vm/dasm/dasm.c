#include "cli.h"
#include "dasm.h"
#include "utils.h"
#include "../program-loader/program-loader.h"
#include "../vm-core/vm-definitions.h"
#include "../vm-core/vm-core.h"
#include <stdbool.h>
#include <stdio.h>


/*********************
 * Private Functions *
 *********************/
static bool
vmdasm_dasm(struct darr * program, FILE * outstream)
{
    if (!vmdasm_ensure_ptr(program)) return false;
    if (!vmdasm_ensure_ptr(outstream)) return false;
    for (int i = 0; i < darr_size(program); i++) {
        union vm_instr_view * view = darr_get(program, i);
        fprintf(outstream, "%6s", vm_op_to_str(view->base.op));
        switch (view->base.op) {
            /* Move op */
        case VM_MOV: {
            if (view->mov.flag & VM_MOV_CONST_TO_REG) {
                int dest = view->mov.dest;
                i++;
                union vm_instr_view * const_view = darr_get(program, i);
                int val = const_view->raw;
                fprintf(outstream, "\tr%d\t%d", dest, val);
            } else {
                int dest = view->mov.dest;
                int src = view->mov.src;
                fprintf(outstream, "\tr%d\tr%d", dest, src);
            }
            break;
        }
            /* Binary ops */
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIV:
        case VM_MOD:
        case VM_AND:
        case VM_OR:
        case VM_XOR:
        case VM_LT:
        case VM_LE:
        case VM_GT:
        case VM_GE:
        case VM_NE:
        case VM_EQ:
            fprintf(outstream, "\tr%d\tr%d\tr%d",
                    view->bin.dest,
                    view->bin.arg1,
                    view->bin.arg2);
            break;
            /* Unary ops */
        case VM_NEG:
        case VM_NOT:
            fprintf(outstream, "\tr%d\tr%d",
                    view->un.dest,
                    view->un.arg);
            break;
            /* Memory operations */
        case VM_LOAD:
            fprintf(outstream, "\tr%d\t%d", view->load.dest_reg, view->store.ofst_reg);
            break;
        case VM_STORE:
            fprintf(outstream, "\tr%d\t%d", view->store.ofst_reg, view->store.src_reg);
            break;
        case VM_PRNT: {
            fprintf(outstream, "\tr%d", view->print.reg);
            break;
        }
        case VM_CJMP: {
            fprintf(outstream, "\tr%d\t%d\n", view->cjmp.cond_reg, view->cjmp.loc_reg);
        }
        case VM_JMP: {

        }
        case VM_EXIT: break;
        case VM_ERR: break;
        }
        fprintf(outstream, "\n");
    }
    return true;
}


/**************
 * Public API *
 **************/
bool
vmdasm_main(int argc, char **argv)
{
    struct vmdasm_cli_opts opts = {.err = true};
    struct darr * program = NULL;
    FILE * outstream = NULL;

    /* Parse options */
    opts = vmdasm_cli_getopts(argc, argv);
    if (opts.err) goto error;

    /* Open output stream */
    if (!(outstream = vmdasm_fopen(opts.output_path, "w", stdout))) goto error;

    /* Load program */
    if (!(program = vmprog_ldr_load(opts.input_path))) goto error;

    /* printf("[VM DASM]: Loaded program\n"); */
    vmdasm_dasm(program, outstream);

    /* Exit */
    darr_destroy(&program);
    return true;

error:
    darr_destroy(&program);
    return false;
}
