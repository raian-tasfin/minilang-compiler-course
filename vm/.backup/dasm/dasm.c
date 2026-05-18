#include "cli.h"
#include "../program-loader/program-loader.h"
#include "../vm/vm.h"
#include <stdbool.h>
#include <stdarg.h>


/*******************
 * Private Methods *
 *******************/
static void *
vmdasm_ensure_ptr_proc(void * ptr, char * name)
{
    if (!name) name = "";
    if (ptr) return ptr;
    fprintf(stderr, "[VM DASM]: Null pointer provided for \"%s\"\n", name);
    return NULL;
}
#define vmdasm_ensure_ptr(name)                  \
    (vmdasm_ensure_ptr_proc((name), #name))


void
vmdasm_fprintf(struct vmrun_program_ctx * ctx, const char *fmt, ...)
{
    if (!ctx || !ctx->) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->rprt, fmt, args);
    va_end(args);
}

static bool
vmdasm_dasm(struct vmrun_program_ctx * ctx)
{
    if (!vmdasm_ensure_ptr(ctx)) return false;
    if (!vmdasm_ensure_ptr(ctx->instream)) return false;
    if (!vmdasm_ensure_ptr(ctx->program.buff)) return false;

    bool ok = true;
    for (int i = 0; i < ctx->program.size; i++) {
        union vm_instr_view view = ctx->program.buff[i];
        switch (view.base.op) {
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


/**************
 * Public API *
 **************/
bool
vmdasm_main(int argc, char ** argv)
{
    /* Get command line options */
    struct vmdasm_cli_opts opts = vmdasm_cli_get_opts(argc, argv);
    if (opts.err) return false;

    /* Load program */
    struct vmrun_cli_opts runopts = {
        .input_path = opts.input_path,
        .output_path = opts.output_path,
        .err = opts.err
    };
    struct vmrun_program_ctx ctx = vmrun_program_load(runopts);
    if (ctx.err) return false;

    /* Print Program */

    return true;
}
