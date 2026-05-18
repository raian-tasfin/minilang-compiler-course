#include "run.h"
#include "cli.h"
#include "program-loader.h"
#include "../vm/vm.h"
#include <stdbool.h>


/*******************
 * Private Methods *
 *******************/
static void *
vmrun_ensure_ptr_proc(void * ptr, char * name)
{
    if (!name) name = "";
    if (ptr) return ptr;
    fprintf(stderr, "[VM]: Null pointer provided for \"%s\"\n", name);
    return NULL;
}
#define vmrun_ensure_ptr(name)                  \
    (vmrun_ensure_ptr_proc((name), #name))


/**************
 * Public API *
 **************/
bool
vmrun_main(int argc, char ** argv)
{
    /* Get command line options */
    struct vmrun_cli_opts opts = vmrun_cli_get_opts(argc, argv);
    if (opts.err) return false;

    /* Load program */
    struct vmrun_program_ctx ctx = vmrun_program_load(opts);
    if (ctx.err) return false;

    /* Spin Up VM */
    struct vm * vm = vm_init(opts.output_path, ctx.program.buff);
    if (!vm_run(vm)) return false;
    return true;
}
