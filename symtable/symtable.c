#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "../darr/darr.h"

struct sym_scope {
    struct sym_scope * parent;
    struct darr * children; // array of sym_scope *
    struct darr * arr;      // array of struct symbol *
    int * id;
};


struct sym_scope *
sym_scope_new(struct sym_scope * parent_scope)
{
    struct sym_scope *  new = malloc(sizeof(struct sym_scope));
    *new = (struct sym_scope) {
        .parent = parent_scope,
        .arr = darr_init(sizeof(struct symbol *)),
        .id = parent_scope ? parent_scope->id : malloc(sizeof(int)),
        .children = darr_init(sizeof(struct sym_scope *)),
    };
    if (!parent_scope) *(new->id) = 0;
    if (parent_scope) darr_push_back(parent_scope->children, &new);
    return new;
}


struct symbol *
sym_scope_find_local(struct sym_scope * scope, char * name)
{
    if (!scope || !name) return NULL;
    int n = darr_size(scope->arr);
    for (int i = 0; i < n; i++) {
        struct symbol ** sym = darr_get(scope->arr, i);
        if (strcmp(sym[0]->name, name) == 0) return *sym;
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
sym_new(struct sym_scope * scope, char * name, enum scalar_type type)
{
    if (!scope) return NULL;
    struct symbol * sym = malloc(sizeof(struct symbol));
    *sym = (struct symbol) {
        .id = (*scope->id)++,
        .name = (name != NULL) ? strdup(name) : NULL,
        .type = type,
    };
    darr_push_back(scope->arr, &sym);
    return sym;
}

enum scalar_type
sym_type(struct symbol * sym)
{
    return sym->type;
}

// meant to be called by scope delete only
static void
sym_delete(struct symbol * sym)
{
    if (!sym) return;
    free(sym->name);
}


void
sym_scope_delete(struct sym_scope * scope)
{
    if (!scope) return;
    /* struct sym_scope * parent; */
    // skip. we are passing down recursion

    /* struct darr * children; // array of sym_scopes */
    if (scope->children) {
        int n = darr_size(scope->children);
        for (int i = 0; i < n; i++) {
            struct sym_scope ** child = darr_get(scope->children, i);
            sym_scope_delete(*child);
        }
        darr_destroy(&scope->children);
    }
    /* struct darr * arr;      // array of struct symbols */
    if (scope->arr) {
        int n = darr_size(scope->arr);
        for (int i = 0; i < n; i++) {
            struct symbol ** sym = darr_get(scope->arr, i);
            sym_delete(*sym);
        }
        darr_destroy(&scope->arr);
    }
    /* int * id; */
    if (!scope->parent) free(scope->id);
}

struct sym_scope *
sym_scope_child(struct sym_scope * scope, int n)
{
    return *(struct sym_scope**)darr_get(scope->children, n);
}
