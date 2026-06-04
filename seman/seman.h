#include "../ast/ast.h"
#include <stdbool.h>

#ifndef SEMAN_H
#define SEMAN_H 1

bool seman_type_check(struct ast_node * root);

#endif
// SEMAN_H
