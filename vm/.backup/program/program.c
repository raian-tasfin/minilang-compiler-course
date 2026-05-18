#include "program.h"
#include <stdio.h>
#include <stdlib.h>



/*********************
 * Private Utilities *
 *********************/
static bool
vmrun_ensure_ptr(void * p, char * name)
{
    if (p) return true;
    fprintf(stderr, "[VM RUN]: NULL passed for pointer %s\n", name);
    return false;
}

static void *
vmrun_realloc(void * ptr, int size, char * name)
{
    void * res = realloc(ptr, size);
    if (res) return res;
    fprintf(stderr, "[VM RUN]: Failed to realloc %d bytes for %s\n", size, name);
    return NULL;
}



/**************
 * Public API *
 **************/
struct vmrun_program
vmrun_program_init(void)
{
    struct vmrun_program program =  {
        .buff = malloc(sizeof(union vm_instr_view) * 10),
        .cap = 10,
        .size = 0
    };
    if (!program.buff) {
        fprintf(stderr, "[VM RUN]: Failed to allocate buffer for loading program.\n");
        goto error;
    }
    return program;

 error:
    if (program.buff) free(program.buff);
    program.err = true;
    return program;
}


bool
vmrun_push_back(struct vmrun_program * program, union vm_instr_view view)
{
    if (!vmrun_ensure_ptr(program, "program")) return false;
    if (!vmrun_ensure_ptr(program->buff, "program buffer")) return false;

    if (program->size == program->cap) {
        int new_cap = program->cap * 2;
        void * new_buff = vmrun_realloc(program->buff, new_cap * sizeof(program->buff[0]), "program buffer");
        if (!new_buff) return false;
        program->cap = new_cap;
        program->buff = new_buff;
    }

    program->buff[program->size++] = view;
    return true;
}

void
vmrun_destroy(struct vmrun_program * program)
{
    if (!program) return;
    free((*program).buff);
}
