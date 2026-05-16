#ifndef VM_CODEGEN_H
#define VM_CODEGEN_H 1

#include "vm.h"
#include <stdint.h>

/*
 * Expected usage:
 * union vm_instr_view program[] = {
 *     VM_GENARR_MOV(1, VM_MOV_CONST_TO_REG, 416354),
 *     VM_GENARR_ADD(1, 2, 3),
 * };
 */

#define VM_GENARR_MOV(d, v, f)                  \
    ((union vm_instr_view){                     \
        .mov = {                                \
            .op   = VM_MOV,                     \
            .dest = (d),                        \
            .flag = (f)                         \
        }                                       \
    }),                                         \
        ((union vm_instr_view){ .raw = (v) })


#define VM_GENARR_EXIT()                        \
    ((union vm_instr_view){                     \
        .base.op = VM_EXIT                      \
    })


#define VM_GENARR_PRINT(s)                      \
    ((union vm_instr_view){                     \
        .print = {                              \
            .op  = VM_PRNT,                     \
            .reg = (s),                         \
        }                                       \
    })


#define VM_GENARR_BIN(o, d, a1, a2)             \
    ((union vm_instr_view){                     \
        .bin = {                                \
            .op   = (o),                        \
            .dest = (d),                        \
            .arg1 = (a1),                       \
            .arg2 = (a2)                        \
        }                                       \
    })

#define VM_GENARR_ADD(d, a1, a2) VM_GENARR_BIN(VM_ADD, (d), (a1), (a2))
#define VM_GENARR_SUB(d, a1, a2) VM_GENARR_BIN(VM_SUB, (d), (a1), (a2))
#define VM_GENARR_MUL(d, a1, a2) VM_GENARR_BIN(VM_MUL, (d), (a1), (a2))
#define VM_GENARR_DIV(d, a1, a2) VM_GENARR_BIN(VM_DIV, (d), (a1), (a2))
#define VM_GENARR_MOD(d, a1, a2) VM_GENARR_BIN(VM_MOD, (d), (a1), (a2))

#endif
// VM_CODEGEN_H
