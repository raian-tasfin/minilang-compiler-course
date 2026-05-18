#include "../util/util.h"

/**************************************
 * Program Loader Utilities from Util *
 **************************************/
#define vmprog_ldr_ensure_ptr_proc(ptr, name)           \
    vm_ensure_ptr_proc("VM PROG LOADER", (ptr), (name))

#define vmprog_ldr_ensure_ptr(name)             \
    vm_ensure_ptr("VM PROG LOADER", (name))

#define vmprog_ldr_fopen(p, m, d)                                   \
    vm_fopen("VM PROG LOADER", p, m, d)
