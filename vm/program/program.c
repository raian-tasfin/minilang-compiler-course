#include "program.h"
#include "utils.h"
#include <stdlib.h>

struct vmprog_program *
vmprog_init(void)
{
    struct vmprog_program * program = NULL;
    if (!(vmprog_malloc(sizeof(struct vmprog_program), "Program"))) return NULL;

    *program = (struct vmprog_program){0};
    return program;
}


bool
vmprog_push_back(struct vmprog_program * prog, union vm_instr_view view)
{
    if (!vmprog_ensure_ptr(prog)) return false;
    if (prog->size == prog->cap) {
        int new_cap = prog->cap * 2 + 1;
        void * new_buff = NULL;
        if (!(new_buff = vmprog_realloc(prog->buff, new_cap, "Program Buffer"))) return false;
        prog->cap = new_cap;
        prog->buff = new_buff;
    }
    prog->buff[prog->size++] = view;
    return true;
}


bool
vmprog_destroy(struct vmprog_program ** program)
{
    if (!program || !*program) return true;
    if (program[0]->buff) free(program[0]->buff);
    free(*program);
    *program = NULL;
    return true;
}
