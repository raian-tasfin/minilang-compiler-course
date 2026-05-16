#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#ifndef VM_H
#define VM_H 1

#define VM_REGISTER_CNT 256


/****************
 * Error Status *
 ****************/
enum
vm_err {
    VM_OK,
    VM_FAIL,
};


/***********
 * Opcodes *
 ***********/
enum
vm_op : uint8_t
{
    VM_ERR = 0,
    VM_MOV,
    VM_ADD,
    VM_SUB,
    VM_MUL,
    VM_DIV,
    VM_MOD,
    VM_PRNT,
    VM_EXIT,
};


/*********************
 * Instruction Types *
 *********************/
/* Each type should take no more than 4 bytes. The combined version
 * should be  4 bytes long.
 */
struct vm_instruction {
    enum vm_op op: 8;
    uint32_t buff : 24;
};

/* Move Instruction */
enum vm_mov_flag : uint16_t {
    VM_MOV_CONST_TO_REG = 1
};
struct vm_mov_instruction {
    enum vm_op op: 8;
    uint8_t dest: 8;
    enum vm_mov_flag flag : 16;
};

/* Binary Operation */
struct vm_binop_instruction {
    enum vm_op op: 8;
    uint8_t dest: 8;
    uint8_t arg1: 8;
    uint8_t arg2: 8;
};

/* Print */
struct vm_print_instruction {
    enum vm_op op: 8;
    uint8_t reg: 8;
    uint16_t flags: 16;
};

/* Combinede */
union vm_instr_view {
    struct vm_instruction       base;
    struct vm_mov_instruction   mov;
    struct vm_binop_instruction bin;
    struct vm_print_instruction print;
    uint32_t                    raw;
};


/***********
 * Context *
 ***********/
struct vm {
    /* Context to run */
    struct {
        FILE * print_stream;
    } ctx;

    /* Machine state */
    struct {
        union vm_instr_view * program;
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
 *                 this library. The provider is responsible for this.
 */
struct vm *
vm_init(char * print_path, union vm_instr_view * program);

bool
vm_destroy(struct vm * vm);

bool
vm_run(struct vm * vm);

#endif
// VM_H
