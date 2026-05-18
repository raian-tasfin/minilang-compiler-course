#include "cli.h"
#include "run.h"
#include <stdbool.h>
#include <stdio.h>

bool
vmrun_main(int argc, char **argv)
{
    /* Parse CLI options */
    struct vmrun_cli_opts opts = vmrun_cli_getopts(argc, argv);
    if (opts.err) return false;
    fprintf(stdout,
            "[VM RUN]: Parsed CLI options.\n"
            "          input  path: %s\n"
            "          output path: %s\n",
            opts.input_path ? opts.input_path : "NULL (ERROR: Input path must be present)",
            opts.output_path ? opts.output_path : "stdout"
            );

    return true;
}
