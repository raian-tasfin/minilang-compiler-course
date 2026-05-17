#include "cli.h"
#include "program.h"

#ifndef VMRUN_PROGRAM_LOADER_H
#define VMRUN_PROGRAM_LOADER_H 1


struct vmrun_program_ctx {
    struct vmrun_program program;
    FILE * instream;
    bool err;
};


struct vmrun_program_ctx
vmrun_program_load(struct vmrun_cli_opts opts);


#endif
// VMRUN_PROGRAM_LOADER_H
