#include "../util/util.h"

#ifndef VM_CORE_UTILS_H
#define VM_CORE_UTILS_H 1


#define vmcr_ensure_ptr_proc(p, n)              \
    vm_ensure_ptr_proc("VM CORE", (p), (n))


#define vmcr_ensure_ptr( n)                     \
    vm_ensure_ptr("VM CORE", (n))


#define vmcr_malloc(s, r)                       \
    vm_malloc("VM CORE", (s), (r))


#define vmcr_fopen(p, m, d)                     \
    vm_fopen("VM CORE", (p), (m), (d))


#define vmcr_fclose(s)                          \
    vm_fclose("VM CORE", (s))


#define vmcr_err(f, ...)                        \
    vm_err("VM CORE", f, __VA_ARGS__)


#endif
// VM_CORE_UTILS_H
