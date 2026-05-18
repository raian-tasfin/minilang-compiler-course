#include "cli.h"
#include "../run/run.h"
#include "../run/cli.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void
vmcli_help(void)
{
    fprintf(
            stdout,
            "usage:\n"
            "    vm <subcommand> [options]\n\n"
            "subcommands:\n"
            "    run     run a program\n"
            );
}

bool
vmcli_main(int argc, char **argv)
{
    if (argc < 2) {
        vmcli_help();
        return false;
    }
    if (strcmp(argv[1], "run") == 0) {
        return vmrun_main(argc - 1, argv + 1);
    }
    vmcli_help();
    return false;
}
