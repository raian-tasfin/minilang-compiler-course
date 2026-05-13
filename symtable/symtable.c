#include "symtable.h"
#include <stdlib.h>
#include <string.h>


/***********************
 * Private Definitions *
 ***********************/
struct symrec {
    char * name;
    struct symrec * next;
    int type;
};

struct scope {
    struct symrec * head;
    struct scope * parent;
};

static struct scope *
scope_new(struct symrec * head, struct scope * parent)
{
    struct scope * new = malloc(sizeof(struct scope));
    *new = (struct scope){
        .head = head,
        .parent = parent
    };
    return new;
}

void
symrec_destroy(struct symrec ** sym)
{
    if (!sym) return;
    while (*sym) {
        struct symrec * tmp = *sym;
        *sym = sym[0]->next;
        if (tmp->name) free(tmp->name);
        free(tmp);
    }
    *sym = NULL;
}

static struct symrec *
symrec_inscope(struct scope *scope, char *name)
{
    if (!scope) return NULL;
    if (!name) return NULL;
    struct symrec *rec = scope->head;
    while (rec) {
        if (strcmp(rec->name, name) == 0)
            return rec;
        rec = rec->next;
    }
    return NULL;
}

static struct symrec *
symrec_new(char * name, struct symrec * next, int type)
{
    struct symrec * new = malloc(sizeof(struct symrec));
    *new = (struct symrec){
        .name = strdup(name),
        .next = next,
        .type = type
    };
    return new;
}


/**************
 * Public API *
 **************/
struct scope *
scope_enter_new(struct scope * curr)
{
    return scope_new(NULL, curr);
}

struct scope *
scope_exit(struct scope * curr)
{
    if (!curr) return NULL;
    struct scope * ret = curr->parent;
    symrec_destroy(&curr->head);
    free(curr);
    return ret;
}


struct symrec *
symbol_lookup(struct scope * scope, char * name)
{
    if (!name) return NULL;
    while (scope) {
        struct symrec * res = symrec_inscope(scope, name);
        if (res) return res;
        scope = scope->parent;
    }
    return NULL;
}

struct symrec *
symbol_insert(struct scope * scope, char * name, int type)
{
    if (!scope) return NULL;
    if (!name) return NULL;
    if (symrec_inscope(scope, name)) return NULL;
    return scope->head = symrec_new(name, scope->head, type);
}
