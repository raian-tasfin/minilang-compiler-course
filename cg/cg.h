#include "../intrep/interp.h"

#ifndef CG_H
#define CG_H 1

struct cg_ctx;

struct cg_ctx * cg_ctx_init(struct ir_block * block);
void cg_ctx_destroy(struct cg_ctx ** ctx);

// cg_darr of type vm_instr_view
struct cg_darr * cg_generate_code(struct cg_ctx * ctx);


#endif
// CG_H
