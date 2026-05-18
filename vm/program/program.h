#include "../vm-core/vm-core.h"
#include "utils.h"
#include <stdbool.h>


#ifndef VM_PROGRAM_H
#define VM_PROGRAM_H 1


/********************
 * Public Interface *
 ********************/
struct vmprog_program {
    union vm_instr_view * buff;
    int size;
    int cap;
};

struct vmprog_program * vmprog_init(void);
bool vmprog_push_back(struct vmprog_program * prog, union vm_instr_view view);
bool vmprog_destroy(struct vmprog_program ** program);


#endif
// VM_PROGRAM_H
