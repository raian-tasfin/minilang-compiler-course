#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "../darr/darr.h"

struct sym_scope {
    struct sym_scope * parent;
    struct darr * arr; // array of struct symbols
    int * id;
};


struct sym_scope *
sym_scope_new(struct sym_scope * parent_scope)
{
    struct sym_scope *  new = malloc(sizeof(struct sym_scope));
    *new = (struct sym_scope) {
        .parent = parent_scope,
        .arr = darr_init(sizeof(struct symbol)),
        .id = parent_scope ? parent_scope->id : malloc(sizeof(int))
    };
    return new;
}


struct symbol *
sym_scope_find_local(struct sym_scope * scope, char * name)
{
    if (!scope || !name) return NULL;
    int n = darr_size(scope->arr);
    for (int i = 0; i < n; i++) {
        struct symbol * sym = darr_get(scope->arr, i);
        if (strcmp(sym->name, name) == 0) return sym;
    }
    return NULL;
}

struct symbol *
sym_scope_find(struct sym_scope * scope, char * name)
{
    if (!name) return NULL;
    while (scope) {
        struct symbol * sym = sym_scope_find_local(scope, name);
        if (sym) return sym;
        scope = scope->parent;
    }
    return NULL;
}

struct symbol *
sym_new(struct sym_scope * scope, char * name, enum sym_scalar_type type)
{
    if (!scope || !name) return NULL;
    struct symbol * sym = malloc(sizeof(struct symbol));
    *sym = (struct symbol) {
        .id = (*scope->id)++,
        .name = name,
        .type = type,
    };
    darr_push_back(scope->arr, sym);
    return sym;
}

enum sym_scalar_type
sym_type(struct symbol * sym)
{
    return sym->type;
}
