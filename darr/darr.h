#include <stdbool.h>

#ifndef CG_DARR_H
#define CG_DARR_H 1

struct cg_darr;
struct cg_darr * cg_darr_init(int elem_size);
void cg_darr_destroy(struct cg_darr ** darr);
bool cg_darr_reserve(struct cg_darr * darr, int cap);
bool cg_darr_resize(struct cg_darr * darr, int new_size, void * fill_src);
bool cg_darr_push_back(struct cg_darr * darr, void * src);
void * cg_darr_get(struct cg_darr * darr, int indx);
bool cg_darr_set(struct cg_darr * darr, int indx, void * src);
bool cg_darr_pop_back(struct cg_darr * darr, void * dst);
int cg_darr_size(struct cg_darr * darr);
bool cg_darr_ensure_index(struct cg_darr *darr, int indx, void *fill_src);

#endif
// CG_DARR_H
