#include "../util/util.h"
#include <stdio.h>

#define vmdasm_fopen(p, m, d)                   \
    vm_fopen("VM DASM", p, m, d)

#define vmdasm_ensure_ptr_proc(p, n)            \
    vm_ensure_ptr_proc("VM DASM", p, n)

#define vmdasm_ensure_ptr(n)                    \
    vm_ensure_ptr("VM DASM", n)
