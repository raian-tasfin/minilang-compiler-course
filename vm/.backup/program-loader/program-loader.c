#include "program-loader.h"
#include "../program/program.h"
#include <stdio.h>


/*********************
 * Private Utilities *
 *********************/
static FILE *
vmrun_fopen(char * path, char * mode, FILE * default_stream)
{
    if (!path) return default_stream;
    FILE * f = NULL;
    if ((f = fopen(path, mode))) return f;
    fprintf(stderr, "[VM RUN]: Error opening file \"%s\"\n", path);
    return NULL;
}

static bool
vmrun_fclose(FILE * stream)
{
    if (!stream) return true;
    if (stream == stdin) return true;
    if (stream == stdout) return true;
    if (stream == stderr) return true;
    int stat = fclose(stream);
    if (stat == 0) return true;
    fprintf(stderr, "[VM RUN]: Error closing file stream.\n");
    return false;
}



/**************
 * Public API *
 **************/
struct vmrun_program_ctx
vmrun_program_load(struct vmrun_cli_opts opts)
{
    struct vmrun_program_ctx ctx = {0};

    if (!(ctx.instream = vmrun_fopen(opts.input_path, "rb", NULL))) {
        ctx.err = true;
        return ctx;
    }

    ctx.program = vmrun_program_init();
    if (ctx.program.err) {
        ctx.err = true;
        return ctx;
    }

    union vm_instr_view view;
    while (fread(&view, sizeof(view), 1, ctx.instream) == 1) {
        if (!vmrun_push_back(&ctx.program, view)) {
            fprintf(stderr, "[VM RUN]: Failed to buffer instruction from \"%s\"\n",
                    opts.input_path);
            vmrun_fclose(ctx.instream);
            vmrun_destroy(&ctx.program);
            ctx.err = true;
            return ctx;
        }
    }

    if (ferror(ctx.instream)) {
        fprintf(stderr, "[VM RUN]: Read error on file \"%s\"\n", opts.input_path);
        vmrun_fclose(ctx.instream);
        vmrun_destroy(&ctx.program);
        ctx.err = true;
        return ctx;
    }

    vmrun_fclose(ctx.instream);
    ctx.instream = NULL;
    return ctx;
}
