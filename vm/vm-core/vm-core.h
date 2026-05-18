#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "../program/program.h"


#ifndef VM_CORE_H
#define VM_CORE_H 1




/***********
 * Context *
 ***********/
#define VM_REGISTER_CNT 256

struct vm {
    /* Context to run */
    struct {
        FILE * print_stream;
    } ctx;

    /* Machine state */
    struct {
        struct vmprog_program * program;
        int32_t r[VM_REGISTER_CNT];
        uint32_t ip;
    } state;
};


/*
 * Initialize `machine state' and `run context'.
 * Arguments:
 *     print_path: used to open FILE * print_stream that PRNT
 *                 operation prints to. Keepn NULL if you want
 *                 print_stream to be stdout.
 *     program:    pointer to the program to run. This is not freed by
 *                 this library. The provider is responsible for that.
 */
struct vm *
vm_init(char * print_path, struct vmprog_program * program);


bool
vm_destroy(struct vm * vm);


bool
vm_run(struct vm * vm);



#endif
// VM_CORE_H
