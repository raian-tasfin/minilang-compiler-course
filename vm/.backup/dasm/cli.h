#include <stdbool.h>

#ifndef VMDASM_CLI_H
#define VMDASM_CLI_H 1


struct vmdasm_cli_opts {
    char * input_path;
    char * output_path;
    bool err;
};


struct vmdasm_cli_opts
vmdasm_cli_get_opts(int argc, char ** argv);

void
vmdasm_cli_help(void);


#endif
// VMDASM_CLI_H
