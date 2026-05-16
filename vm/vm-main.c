#include <stdio.h>
#include "vm.h"
#include "vm-codegen.h"

int main()
{
    union vm_instr_view prog[] = {
        VM_GENARR_MOV(  1,  265, VM_MOV_CONST_TO_REG),    // tmp_1 =   265
        VM_GENARR_MOV(  6,  120, VM_MOV_CONST_TO_REG),    // tmp_6 =   120
        VM_GENARR_MOV(  7,   54, VM_MOV_CONST_TO_REG),    // tmp_7 =    54
        VM_GENARR_ADD(  5,    6,                   7),    // tmp_5 = tmp_6 + tmp_7
        VM_GENARR_MOV(  8,   87, VM_MOV_CONST_TO_REG),    // tmp_8 =    87
        VM_GENARR_SUB(  4,    5,                    8),   // tmp_4 = tmp_5 - tmp_8
        VM_GENARR_MOV(  9,   97, VM_MOV_CONST_TO_REG),    // tmp_9 =    97
        VM_GENARR_ADD(  3,    4,                   9),    // tmp_3 = tmp_4 + tmp_9
        VM_GENARR_MOV( 11, 5468, VM_MOV_CONST_TO_REG),    // tmp_11 =  5468
        VM_GENARR_MOV( 12,   54, VM_MOV_CONST_TO_REG),    // tmp_12 =    54
        VM_GENARR_MUL( 10,   11,                  12),    // tmp_10 = tmp_11 * tmp_12
        VM_GENARR_SUB(  2,    3,                  10),    // tmp_2 = tmp_3 - tmp_10
        VM_GENARR_ADD(  0,    1,                   2),    // tmp_0 = tmp_1 + tmp_2
        VM_GENARR_PRINT(0),
        VM_GENARR_EXIT(),
    };

    struct vm * vm = vm_init(NULL, prog);
    vm_run(vm);
    vm_destroy(vm);

    return 0;
}
