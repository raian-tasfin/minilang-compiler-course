#include "../vm/vm.h"
#include <stdbool.h>


#ifndef VMRUN_PROGRAM_H
#define VMRUN_PROGRAM_H 1


struct vmrun_program {
    union vm_instr_view * buff;
    int cap;
    int size;
    bool err;
};

struct vmrun_program
vmrun_program_init(void);

bool
vmrun_push_back(struct vmrun_program * program, union vm_instr_view view);


void
vmrun_destroy(struct vmrun_program * program);


#endif
// VMRUN_PROGRAM_H
