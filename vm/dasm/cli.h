#include <stdbool.h>


#ifndef VM_DASM_CLI_H
#define VM_DASM_CLI_H 1


struct vmdasm_cli_opts {
    char * input_path;  // path of .mlo file to deassemble
    char * output_path; // path of .mlasm to write assembly to
    bool err;
};


struct vmdasm_cli_opts
vmdasm_cli_getopts(int argc, char **argv);

void
vmdasm_cli_help(void);


#endif
// VM_DASM_CLI_H
