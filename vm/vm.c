#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


/*************
 * Reporting *
 *************/
char *
vm_opcode_to_str(enum vm_op op) {
    switch (op) {
    case VM_MOV: return "VM_MOV";
    case VM_ADD: return "VM_ADD";
    case VM_SUB: return "VM_SUB";
    case VM_MUL: return "VM_MUL";
    case VM_DIV: return "VM_DIV";
    case VM_MOD: return "VM_MOD";
    case VM_PRNT: return "VM_PRNT";
    case VM_EXIT: return "VM_EXIT";
    case VM_ERR: return "VM_ERR";
    default:     return "VM_UNKOWN";
    }
}


/*****************************
 * Private: Ensure Resources *
 *****************************/
static void *
vm_ensure_ptr_proc(void * ptr, char * name)
{
    if (!name) name = "";
    if (ptr) return ptr;
    fprintf(stderr, "[VM]: Null pointer provided for \"%s\"\n", name);
    return NULL;
}
#define vm_ensure_ptr(name)                     \
    (vm_ensure_ptr_proc((name), #name))

static bool
vm_ensure_vm(struct vm * vm) {
    if (!vm_ensure_ptr(vm)) return false;
    if (!vm_ensure_ptr(vm->ctx.print_stream)) return false;
    if (!vm_ensure_ptr(vm->state.program)) return false;
    return true;
}


static bool
vm_ensure_op_dispatch(union vm_instr_view view, enum vm_op op)
{
    if (view.base.op == op) return true;
    fprintf(stderr, "[VM]: Wrong dispatch for op '%s'.\n", vm_opcode_to_str(op));
    return false;
}


/******************************
 * Private: General Utilities *
 ******************************/
static void *
vm_malloc(int size, char * resource_name)
{
    if (!(vm_ensure_ptr(resource_name))) return NULL;
    void * rec = NULL;
    if ((rec = malloc(size))) return rec;
    fprintf(stderr, "[VM]: Failed allocating for \"%s\"\n", resource_name);
    return rec;
}

static FILE *
vm_fopen(char * path, char * mode, FILE * default_stream)
{
    if (!path) return default_stream;
    FILE * f = NULL;
    if ((f = fopen(path, mode))) return f;
    fprintf(stderr, "[VM]: Error opening file \"%s\"\n", path);
    return NULL;
}

static bool
vm_fclose(FILE * stream)
{
    if (!stream) return true;
    if (stream == stdin) return true;
    if (stream == stdout) return true;
    if (stream == stderr) return true;
    int stat = fclose(stream);
    if (stat == 0) return true;
    fprintf(stderr, "[VM]: Error closing file stream.\n");
    return false;
}


/**************************************
 * Private Execution Specific Methods *
 **************************************/
static union vm_instr_view
vm_fetch(struct vm * vm)
{
    union vm_instr_view view = { .base.op = VM_ERR };
    if (!vm_ensure_vm(vm)) return view;
    view = vm->state.program[vm->state.ip++];
    return view;
}


static bool
vm_exec_mov(struct vm * vm, union vm_instr_view view)
{
    if (!vm_ensure_vm(vm)) return false;
    if (!vm_ensure_op_dispatch(view, VM_MOV)) return false;

    /* Ensure move flag */
    if (!(view.mov.flag & VM_MOV_CONST_TO_REG)) {
        fprintf(stderr, "[VM]: Only 'MOV REG CONST' is implemented.\n");
        return EXIT_FAILURE;
    }

    int32_t val = (int32_t)(vm->state.program[vm->state.ip++].raw);
    vm->state.r[view.mov.dest] = val;
    return true;
}


static bool
vm_exec_bin(struct vm * vm, union vm_instr_view view)
{
    if (!vm_ensure_vm(vm)) return false;
    switch (view.bin.op) {
    case VM_ADD: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] + vm->state.r[view.bin.arg2]; return true;
    case VM_SUB: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] - vm->state.r[view.bin.arg2]; return true;
    case VM_MUL: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] * vm->state.r[view.bin.arg2]; return true;
    case VM_DIV: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] / vm->state.r[view.bin.arg2]; return true;
    case VM_MOD: vm->state.r[view.bin.dest] = vm->state.r[view.bin.arg1] % vm->state.r[view.bin.arg2]; return true;
    default: fprintf(stderr, "[VM]: Wrong dispatch for binary op.\n"); return false;
    };
}

static bool
vm_exec_print(struct vm * vm, union vm_instr_view view)
{
    if (!vm_ensure_vm(vm)) return false;
    if (!vm_ensure_op_dispatch(view, VM_PRNT)) return false;
    fprintf(vm->ctx.print_stream, "%d\n", vm->state.r[view.print.reg]);
    return true;
}


/**************
 * Public API *
 **************/
struct vm *
vm_init(char * print_path, union vm_instr_view * program)
{
    if (!vm_ensure_ptr(program)) return NULL;
    struct vm * vm = NULL;
    if (!(vm = vm_malloc(sizeof(struct vm), "VM"))) return NULL;
    *vm = (struct vm) {
        .ctx.print_stream = vm_fopen(print_path, "w", stdout),
        .state.program = program,
    };
    return vm;
}

bool
vm_destroy(struct vm * vm)
{
    if (vm_fclose(vm->ctx.print_stream)) return false;
    return true;
}

bool
vm_run(struct vm * vm)
{
    if (!vm_ensure_ptr(vm)) return false;
    if (!vm_ensure_ptr(vm->ctx.print_stream)) return false;
    if (!vm_ensure_ptr(vm->state.program)) return false;

    bool ok = true;
    while (true) {
        union vm_instr_view view = vm_fetch(vm);
        switch (view.base.op) {
        case VM_MOV:
            if(!(ok = vm_exec_mov(vm, view)))  goto panic;
            break;
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIV:
        case VM_MOD:
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
    fprintf(stderr, "[VM]: Error state. Panic!\n");
cleanup:
    vm_destroy(vm);
    return ok;

}
