#include "program-loader.h"
#include "../program/program.h"
#include "utils.h"
#include <stdbool.h>


struct vmprog_program *
vmprog_ldr_load(char *input_path)
{
    FILE *f = NULL;
    struct vmprog_program *prog = NULL;
    union vm_instr_view instr;

    if (!vmprog_ldr_ensure_ptr(input_path)) goto error;
    if (!(prog = vmprog_init())) goto error;
    if (!(f = vmprog_ldr_fopen(input_path, "rb", NULL))) goto error;
    while (fread(&instr.raw, sizeof(instr.raw), 1, f) == 1) {
        if (!vmprog_push_back(prog, instr))
            goto error;
    }
    if (ferror(f)) goto error;

    fclose(f);
    return prog;

error:
    if (f) fclose(f);
    if (prog) vmprog_destroy(&prog);
    return NULL;
}
