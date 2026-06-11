#include "../ast/ast.h"
#include "../srcbuf/srcbuf.h"
#include <stdbool.h>

#ifndef SEMAN_H
#define SEMAN_H 1

bool seman_type_check(struct ast_node * root, struct src_buffer * sb);

#endif
// SEMAN_H
