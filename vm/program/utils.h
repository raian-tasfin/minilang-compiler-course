#include "../util/util.h"

/*******************************
 * Program Utilities from Util *
 *******************************/
#define vmprog_ensure_ptr_proc(ptr, name)           \
    vm_ensure_ptr_proc("VM PROG", (ptr), (name))

#define vmprog_ensure_ptr(name)                                         \
    vm_ensure_ptr("VM PROG", (name))


#define vmprog_fprintf(f, v, ...)               \
    vm_fprintf((f), (fmt), __VA_ARGS__)

#define vmprog_err(f, ...)                      \
    vm_err("VM PROG", f, __VA_ARGS__)

#define vmprog_malloc(s, r)                     \
    vm_malloc("VM PROG", s, r)


#define vmprog_realloc(p, s, r)                 \
    vm_realloc("VM PROG", p, s, r)
