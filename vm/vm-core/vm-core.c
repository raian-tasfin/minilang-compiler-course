#include "vm-core.h"
#include "vm-definitions.h"
#include "utils.h"
#include "../darr/darr.h"
#include "../boolean/boolean.h"


/*********************
 * Private Utilities *
 *********************/
static bool
vmcr_ensure_vm(struct vm * vm) {
    if (!vmcr_ensure_ptr(vm)) return false;
    if (!vmcr_ensure_ptr(vm->ctx.print_stream)) return false;
    if (!vmcr_ensure_ptr(vm->state.program)) return false;
    return true;
}

static union vm_instr_view
vmcr_fetch(struct vm * vm)
{
    union vm_instr_view view = { .base.op = VM_ERR };
    if (!vmcr_ensure_vm(vm)) return view;
    view = *(union vm_instr_view*)(darr_get(vm->state.program, vm->state.ip++));
    return view;
}

static bool
vmcr_ensure_op_dispatch(union vm_instr_view view, enum vm_op op)
{
    if (view.base.op == op) return true;
    vmcr_err("Wrong dispatch for op '%s'.", vm_op_to_str(op));
    return false;
}


/**************************************
 * Private Execution Specific Methods *
 **************************************/
static bool
vm_exec_mov(struct vm * vm, union vm_instr_view view)
{
    if (!vmcr_ensure_vm(vm)) return false;
    if (!vmcr_ensure_op_dispatch(view, VM_MOV)) return false;

    /* Ensure move flag */
    if (!(view.mov.flag & VM_MOV_CONST_TO_REG)) {
        vmcr_err("%s", "Only 'MOV REG CONST' is implemented.");
        return false;
    }
    int32_t val = (*(union vm_instr_view*)darr_get(vm->state.program, vm->state.ip++)).raw;
    vm->state.r[view.mov.dest] = val;
    return true;
}


static bool
vm_exec_bin(struct vm * vm, union vm_instr_view view)
{
    if (!vmcr_ensure_vm(vm)) return false;
    switch (view.bin.op) {
    case VM_ADD: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  + vm->state.r[view.bin.arg2]; return true;
    case VM_SUB: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  - vm->state.r[view.bin.arg2]; return true;
    case VM_MUL: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  * vm->state.r[view.bin.arg2]; return true;
    case VM_DIV: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  / vm->state.r[view.bin.arg2]; return true;
    case VM_MOD: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  % vm->state.r[view.bin.arg2]; return true;
    case VM_AND: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] && vm->state.r[view.bin.arg2]; return true;
    case VM_OR : vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] || vm->state.r[view.bin.arg2]; return true;
    case VM_XOR: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1]  ^ vm->state.r[view.bin.arg2]; return true;
    default: fprintf(stderr, "[VM]: Wrong dispatch for binary op."); return false;
    };
}

static bool
vm_exec_print(struct vm * vm, union vm_instr_view view)
{
    if (!vmcr_ensure_vm(vm)) return false;
    if (!vmcr_ensure_op_dispatch(view, VM_PRNT)) return false;

    if (view.print.flags & VM_PRNT_BOOLEAN)
        fprintf(vm->ctx.print_stream, "%s\n", bool_to_str(vm->state.r[view.print.reg]));
    else
        fprintf(vm->ctx.print_stream, "%d\n", view.print.reg);

    return true;
}



/**************
 * Public API *
 **************/
char *
vm_op_to_str(enum vm_op op)
{
    switch (op){
    case VM_ERR: return  "ERR";
    case VM_MOV: return  "MOV";
    case VM_ADD: return  "ADD";
    case VM_SUB: return  "SUB";
    case VM_MUL: return  "MUL";
    case VM_DIV: return  "DIV";
    case VM_MOD: return  "MOD";
    case VM_AND: return  "AND";
    case VM_OR : return  "OR";
    case VM_XOR: return  "XOR";
    case VM_PRNT: return "PRNT";
    case VM_EXIT: return "EXIT";
    default: return      "UNKOWN";
    }
}


struct vm *
vm_init(char * print_path, struct darr * program)
{
    if (!vmcr_ensure_ptr(program)) return NULL;
    struct vm * vm = NULL;
    if (!(vm = vmcr_malloc(sizeof(struct vm), "VM"))) return NULL;
    *vm = (struct vm) {
        .ctx.print_stream = vmcr_fopen(print_path, "w", stdout),
        .state.program = program,
    };
    return vm;
}


bool
vm_destroy(struct vm * vm)
{
    if (!vm) return true;
    if (vmcr_fclose(vm->ctx.print_stream)) return false;
    return true;
}


bool
vm_run(struct vm * vm)
{
    if (!vmcr_ensure_ptr(vm)) return false;
    if (!vmcr_ensure_ptr(vm->ctx.print_stream)) return false;
    if (!vmcr_ensure_ptr(vm->state.program)) return false;

    bool ok = true;
    while (true) {
        union vm_instr_view view = vmcr_fetch(vm);
        switch (view.base.op) {
        case VM_MOV:
            if(!(ok = vm_exec_mov(vm, view)))  goto panic;
            break;
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIV:
        case VM_MOD:
        case VM_AND:
        case VM_OR:
        case VM_XOR:
            if (!(ok = vm_exec_bin(vm, view))) goto panic;
            break;
        case VM_PRNT:
            if (!(ok= vm_exec_print(vm, view))) goto panic;
            break;
        case VM_EXIT: goto cleanup;
        case VM_ERR : goto panic;
        };
    }
panic:
    vmcr_err("%s", "Error state. Panic!");
cleanup:
    vm_destroy(vm);
    return ok;

}
