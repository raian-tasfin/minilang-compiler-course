#include "cli.h"
#include "dasm.h"
#include "utils.h"
#include "../program-loader/program-loader.h"
#include "../program/program.h"
#include "../vm-core/vm-definitions.h"
#include "../vm-core/vm-core.h"
#include <stdbool.h>
#include <stdio.h>


/*********************
 * Private Functions *
 *********************/
static bool
vmdasm_dasm(struct vmprog_program * program, FILE * outstream)
{
    if (!vmdasm_ensure_ptr(program)) return false;
    if (!vmdasm_ensure_ptr(outstream)) return false;
    for (int i = 0; i < program->size; i++) {
        fprintf(outstream, "%s", vm_op_to_str(program->buff[i].base.op));
        switch (program->buff[i].base.op) {
        case VM_MOV:
            i++;
            int val = program->buff[i].raw;
            fprintf(outstream, "\tr%d\t%d", program->buff[i - 1].mov.dest, val);
            break;
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIV:
        case VM_MOD:
            fprintf(outstream, "\tr%d\tr%d\tr%d",
                    program->buff[i].bin.dest,
                    program->buff[i].bin.arg1,
                    program->buff[i].bin.arg2);
            break;
        case VM_PRNT:
            fprintf(outstream, "\tr%d", program->buff[i].print.reg);
            break;
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
    struct vmprog_program * program = NULL;
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
    vmprog_destroy(&program);
    return true;

error:
    vmprog_destroy(&program);
    return false;
}
