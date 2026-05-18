#include "cli.h"
#include "run.h"
#include "utils.h"
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

    printf("[VM RUN]: Loaded program\n");

    /* Exit */
    vmprog_destroy(&program);
    return true;

error:
    vmprog_destroy(&program);
    return false;
}
