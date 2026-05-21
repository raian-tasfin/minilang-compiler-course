#include "program-loader.h"
#include "../vm-core/vm-definitions.h"
#include "utils.h"
#include <stdbool.h>


struct darr *
vmprog_ldr_load(char *input_path)
{
    FILE * f = NULL;
    struct darr * prog = NULL;
    union vm_instr_view instr;

    if (!vmprog_ldr_ensure_ptr(input_path)) goto error;
    if (!(prog = darr_init(sizeof(union vm_instr_view)))) goto error;
    if (!(f = vmprog_ldr_fopen(input_path, "rb", NULL))) goto error;
    while (fread(&instr.raw, sizeof(instr.raw), 1, f) == 1) {
        if (!darr_push_back(prog, &instr))
            goto error;
    }
    if (ferror(f)) goto error;

    fclose(f);
    return prog;

error:
    if (f) fclose(f);
    if (prog) darr_destroy(&prog);
    return NULL;
}
