#include <stdbool.h>
#include <stdint.h>

#ifndef DARR_H
#define DARR_H 1

struct darr;
struct darr * darr_init(int elem_size);
void darr_destroy(struct darr ** darr);
bool darr_reserve(struct darr * darr, int cap);
bool darr_resize(struct darr * darr, int new_size, void * fill_src);
bool darr_push_back(struct darr * darr, void * src);
void * darr_get(struct darr * darr, int indx);
bool darr_set(struct darr * darr, int indx, void * src);
bool darr_pop_back(struct darr * darr, void * dst);
int darr_size(struct darr * darr);
bool darr_ensure_index(struct darr *darr, int indx, void *fill_src);
uint8_t * cg_buffer(struct darr * darr);
int darr_elem_size(struct darr * darr);


#endif
// DARR_H
