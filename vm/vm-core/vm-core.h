#include <stdint.h>


#ifndef VM_CORE_H
#define VM_CORE_H 1



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

char * vm_op_to_str(enum vm_op op);


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



#endif
// VM_CORE_H
