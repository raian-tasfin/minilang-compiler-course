#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>

struct bitset;

struct bitset * bitset_ctr(int max_id);
void bitset_destroy(struct bitset * set);
struct bitset * bitset_copy(struct bitset * set);
void bitset_assign(struct bitset * dest, struct bitset * src);

void bitset_insert(struct bitset * set, int id);
void bitset_remove(struct bitset * set, int id);
bool bitset_contains(const struct bitset * set, int id);

/* left U= right */
void bitset_union_assign(struct bitset * left, struct bitset * right);

/* left -= right */
void bitset_difference  (struct bitset * left, struct bitset * right);

bool bitset_equal(const struct bitset * left, const struct bitset * right);

#endif /* BITSET_H */
