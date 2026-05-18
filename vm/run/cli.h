#include <stdbool.h>


#ifndef VM_RUN_CLI_H
#define VM_RUN_CLI_H 1


struct vmrun_cli_opts {
    char * input_path;  // path of .mlo file to run
    char * output_path; // output path of PRNT operations
    bool err;
};


struct vmrun_cli_opts
vmrun_cli_getopts(int argc, char **argv);


bool
vmrun_cli_main(int argc, char **argv);

void
vmrun_cli_help(void);


#endif
// VM_RUN_CLI_H
