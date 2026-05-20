#include "darr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct cg_darr {
    int   size;
    int   cap;
    int   elem_size;
    void *buff;
};

struct cg_darr *
cg_darr_init(int elem_size)
{
    if (elem_size <= 0) {
        fprintf(stderr, "[CG DARR]: Element size must be positive.\n");
        return NULL;
    }

    struct cg_darr *darr = malloc(sizeof(*darr));
    if (!darr) {
        fprintf(stderr, "[CG DARR]: Failed allocating dynamic array.\n");
        return NULL;
    }

    *darr = (struct cg_darr){ .elem_size = elem_size };
    return darr;
}


void
cg_darr_destroy(struct cg_darr **darr)
{
    if (!darr || !*darr) return;
    free((*darr)->buff);
    free(*darr);
    *darr = NULL;
}


bool
cg_darr_reserve(struct cg_darr *darr, int cap)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to reserve.\n");
        return false;
    }
    if (cap <= darr->cap) return true;   /* already have enough room */

    void *new_buff = realloc(darr->buff, (size_t)cap * darr->elem_size);
    if (!new_buff) {
        fprintf(stderr, "[CG DARR]: realloc failed in reserve.\n");
        return false;
    }

    darr->buff = new_buff;
    darr->cap  = cap;
    return true;
}


bool
cg_darr_resize(struct cg_darr *darr, int new_size, void *fill_src)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to resize.\n");
        return false;
    }
    if (new_size < 0) {
        fprintf(stderr, "[CG DARR]: new_size must be non-negative.\n");
        return false;
    }

    if (new_size > darr->cap) {
        int new_cap = darr->cap ? darr->cap * 2 : 8;
        if (new_cap < new_size) new_cap = new_size;
        if (!cg_darr_reserve(darr, new_cap)) return false;
    }

    if (new_size > darr->size) {
        char *base  = darr->buff;
        int   first = darr->size;
        if (fill_src) {
            for (int i = first; i < new_size; i++)
                memcpy(base + (size_t)i * darr->elem_size,
                       fill_src,
                       darr->elem_size);
        } else {
            memset(base + (size_t)first * darr->elem_size,
                   0,
                   (size_t)(new_size - first) * darr->elem_size);
        }
    }
    darr->size = new_size;
    return true;
}

bool
cg_darr_push_back(struct cg_darr *darr, void *src)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to push_back.\n");
        return false;
    }
    if (!src) {
        fprintf(stderr, "[CG DARR]: NULL src provided to push_back.\n");
        return false;
    }
    if (!cg_darr_resize(darr, darr->size + 1, NULL)) {
        fprintf(stderr, "[CG DARR]: Failed to grow array in push_back.\n");
        return false;
    }
    char *buff = darr->buff;
    memcpy(buff + (size_t)(darr->size - 1) * darr->elem_size,
           src,
           darr->elem_size);
    return true;
}

void *
cg_darr_get(struct cg_darr *darr, int indx)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to get.\n");
        return NULL;
    }
    if (indx < 0 || indx >= darr->size) {
        fprintf(stderr,
                "[CG DARR]: Index %d out of range [0, %d).\n",
                indx, darr->size);
        return NULL;
    }

    char *buff = darr->buff;
    return buff + (size_t)indx * darr->elem_size;
}

bool
cg_darr_set(struct cg_darr *darr, int indx, void *src)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to set.\n");
        return false;
    }
    if (!src) {
        fprintf(stderr, "[CG DARR]: NULL src provided to set.\n");
        return false;
    }
    if (indx < 0 || indx >= darr->size) {
        fprintf(stderr,
                "[CG DARR]: Index %d out of range [0, %d).\n",
                indx, darr->size);
        return false;
    }

    char *buff = darr->buff;
    memcpy(buff + (size_t)indx * darr->elem_size, src, darr->elem_size);
    return true;
}


bool
cg_darr_pop_back(struct cg_darr *darr, void *dst)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to pop_back.\n");
        return false;
    }
    if (darr->size == 0) {
        fprintf(stderr, "[CG DARR]: pop_back on empty array.\n");
        return false;
    }
    darr->size--;
    if (dst)
        memcpy(dst,
               (char *)darr->buff + (size_t)darr->size * darr->elem_size,
               darr->elem_size);
    return true;
}


int
cg_darr_size(struct cg_darr * darr)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: Size requested for NULL dynamic array.\n");
        return -1;
    }
    return darr->size;
}

bool
cg_darr_ensure_index(struct cg_darr *darr, int indx, void *fill_src)
{
    if (!darr) {
        fprintf(stderr, "[CG DARR]: NULL array provided to ensure_index.\n");
        return false;
    }
    if (indx < 0) {
        fprintf(stderr, "[CG DARR]: Index must be non-negative.\n");
        return false;
    }
    if (indx < darr->size) return true;
    int target_size = indx + 1;
    if (!cg_darr_resize(darr, target_size, fill_src)) {
        fprintf(stderr, "[CG DARR]: Failed to resize array in ensure_index.\n");
        return false;
    }
    return true;
}
