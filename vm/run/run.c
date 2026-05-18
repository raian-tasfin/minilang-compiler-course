#include "cli.h"
#include "run.h"
#include "utils.h"
#include "../vm-core/vm-core.h"
#include "../program-loader/program-loader.h"
#include "../program/program.h"
#include <stdbool.h>
#include <stdio.h>

bool
vmrun_main(int argc, char **argv)
{
    struct vmrun_cli_opts opts = {.err = true};
    struct vmprog_program * program = NULL;
    FILE * outstream = NULL;

    /* Parse options */
    opts = vmrun_cli_getopts(argc, argv);
    if (opts.err) goto error;

    /* Open output stream */
    if (!(outstream = vmrun_fopen(opts.output_path, "w", stdout))) goto error;

    /* Load program */
    if (!(program = vmprog_ldr_load(opts.input_path))) goto error;

    /* Spin up machine */
    struct vm * vm = NULL;
    if (!(vm = vm_init(opts.output_path, program))) goto error;

    /* Run machine */
    if (!vm_run(vm)) goto error;

    /* Exit */
    vmprog_destroy(&program);
    vm_destroy(vm);
    return true;

error:
    vmprog_destroy(&program);
    vm_destroy(vm);
    return false;
}
