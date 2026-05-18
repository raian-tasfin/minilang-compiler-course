#include "vm-core.h"


char *
vm_op_to_str(enum vm_op op)
{
    switch (op){
    case VM_ERR: return "VM_ERR";
    case VM_MOV: return "VM_MOV";
    case VM_ADD: return "VM_ADD";
    case VM_SUB: return "VM_SUB";
    case VM_MUL: return "VM_MUL";
    case VM_DIV: return "VM_DIV";
    case VM_MOD: return "VM_MOD";
    case VM_PRNT: return "VM_PRNT";
    case VM_EXIT: return "VM_EXIT";
    default: return "VM_UNKOWN";
    }
}
