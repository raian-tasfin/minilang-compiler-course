#include "../ast/ast.h"
#include "../srcbuf/srcbuf.h"
#include <stdbool.h>

#ifndef SEMAN_H
#define SEMAN_H 1

bool seman(struct ast_node * root, struct src_buffer * sb, struct sym_scope * scope);

#endif
// SEMAN_H
