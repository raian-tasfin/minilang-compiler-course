#include <stdbool.h>

#ifndef VMRUN_CLI_H
#define VMRUN_CLI_H 1


struct vmrun_cli_opts {
    char * input_path;
    char * output_path;
    bool err;
};


struct vmrun_cli_opts
vmrun_cli_get_opts(int argc, char ** argv);

void
vmrun_cli_help(void);


#endif
// VMRUN_CLI_H
