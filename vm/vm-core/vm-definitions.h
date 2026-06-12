#include <stdint.h>

#ifndef VM_DEFINITIONS_H
#define VM_DEFINITIONS_H 1

/***********
 * Opcodes *
 ***********/
enum
vm_op : uint8_t {
    VM_ERR = 0,
    VM_MOV,
    VM_ADD,
    VM_SUB,
    VM_MUL,
    VM_DIV,
    VM_MOD,
    VM_AND,
    VM_OR,
    VM_XOR,
    VM_NOT,
    VM_NEG,
    VM_LT,
    VM_LE,
    VM_GT,
    VM_GE,
    VM_NE,
    VM_EQ,
    VM_LOAD,
    VM_STORE,
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

/* Load Instruction */
struct vm_load_instruction {
    enum vm_op op: 8;
    uint8_t dest_reg: 8;
    uint8_t ofst_reg: 8;
};

/* Store Instruction */
struct vm_store_instruction {
    enum vm_op op: 8;
    uint8_t ofst_reg: 8;
    uint8_t src_reg: 8;
};

/* Binary Operation */
struct vm_binop_instruction {
    enum vm_op op: 8;
    uint8_t dest: 8;
    uint8_t arg1: 8;
    uint8_t arg2: 8;
};

/* Unary Operation */
struct vm_unop_instruction {
    enum vm_op op: 8;
    uint8_t dest: 8;
    uint8_t arg: 8;
};


/* Print */
// bit 0 - 0 if integer, 1 if boolean
enum vm_prnt_flag : uint16_t {
    VM_PRNT_BOOLEAN = 0x001,
};

struct vm_print_instruction {
    enum vm_op op: 8;
    uint8_t reg: 8;
    uint16_t flags: 16;
};

/* Combined */
union vm_instr_view {
    struct vm_instruction       base;
    struct vm_mov_instruction   mov;
    struct vm_binop_instruction bin;
    struct vm_unop_instruction  un;
    struct vm_load_instruction  load;
    struct vm_store_instruction store;
    struct vm_print_instruction print;
    uint32_t                    raw;
};



#endif
// VM_DEFINITIONS_H
