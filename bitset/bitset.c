#include "bitset.h"
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t   bset_elem_t;

static inline int bitset_bits(void)
{
    return sizeof(bset_elem_t) * 8;
}

struct bitset {
    bset_elem_t * set;
    int size;
    int max_id;
};


static inline int
bitset_slot_of(int id)
{
    return id / bitset_bits();
}

static inline int
bitset_bit_of (int id)
{
    return id % bitset_bits();
}


static inline bset_elem_t
bitset_mask_of(int id)
{
    return (bset_elem_t)1 << bitset_bit_of(id);
}

static inline bool
bitset_valid_id(const struct bitset * set, int id)
{
    return set && id >= 0 && id <= set->max_id;
}


struct bitset *
bitset_ctr(int max_id)
{
    int size = (max_id + 1 + bitset_bits() - 1) / bitset_bits();

    struct bitset *bset = malloc(sizeof *bset);
    bset->max_id = max_id;
    bset->size = size;
    bset->set = calloc(size, sizeof(bset_elem_t));
    return bset;
}

void
bitset_destroy(struct bitset *set)
{
    if (!set) return;
    free(set->set);
    free(set);
}



struct bitset *
bitset_copy(struct bitset * set)
{
    if (!set) return NULL;
    struct bitset * res = bitset_ctr(set->max_id);
    for (int i = 0; i < set->size; i++) {
        res->set[i] = set->set[i];
    }
    return res;
}


void
bitset_assign(struct bitset * dest, struct bitset * src)
{
    if (!dest || !src) return;
    dest->max_id = src->max_id;
    dest->size = src->size;
    dest->set = realloc(dest->set, src->size * sizeof(bset_elem_t));
    for (int i = 0; i < dest->size; i++) {
        dest->set[i] = src->set[i];
    }
}

void
bitset_insert(struct bitset *set, int id)
{
    if (!bitset_valid_id(set, id)) return;
    set->set[bitset_slot_of(id)] |= bitset_mask_of(id);
}

void
bitset_remove(struct bitset *set, int id)
{
    if (!bitset_valid_id(set, id)) return;
    set->set[bitset_slot_of(id)] &= ~bitset_mask_of(id);
}

bool
bitset_contains(const struct bitset *set, int id)
{
    if (!bitset_valid_id(set, id)) return false;
    return (set->set[bitset_slot_of(id)] & bitset_mask_of(id)) != 0;
}


void
bitset_union_assign(struct bitset *left, struct bitset *right)
{
    if (!left || !right) return;
    int n = left->size < right->size ? left->size : right->size;
    for (int i = 0; i < n; i++) {
        left->set[i] |= right->set[i];
    }
}

void
bitset_difference(struct bitset *left, struct bitset *right)
{
    if (!left || !right) return;
    int n = left->size < right->size ? left->size : right->size;
    for (int i = 0; i < n; i++) {
        left->set[i] &= ~right->set[i];
    }
}

bool
bitset_equal(const struct bitset *left, const struct bitset *right)
{
    if (!left || !right) return false;
    if (left->max_id != right->max_id) return false;
    for (int i = 0; i < left->size; i++) {
        if (left->set[i] != right->set[i]) {
            return false;
        }
    }
    return true;
}
