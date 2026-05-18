#include "cli.h"
#include "dasm.h"
#include <stdbool.h>
#include <stdio.h>

bool
vmdasm_main(int argc, char **argv)
{
    /* Parse CLI options */
    struct vmdasm_cli_opts opts = vmdasm_cli_getopts(argc, argv);
    if (opts.err) return false;
    fprintf(stdout,
            "[VM DASM]: Parsed CLI options.\n"
            "           input  path: %s\n"
            "           output path: %s\n",
            opts.input_path ? opts.input_path : "NULL (ERROR: Input path must be present)",
            opts.output_path ? opts.output_path : "stdout"
            );

    return true;
}
