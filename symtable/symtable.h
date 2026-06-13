#include "../types/scalars.h"

#ifndef SYMTABLE_H
#define SYMTABLE_H 1

struct symbol {
    char * name;
    enum scalar_type type;
    int id;
};

struct sym_scope;

struct sym_scope * sym_scope_new(struct sym_scope * parent_scope);
struct sym_scope * sym_scope_child(struct sym_scope * scope, int n);
struct symbol    * sym_scope_find_local(struct sym_scope * scope, char * name);
struct symbol    * sym_scope_find(struct sym_scope * scope, char * name);
void               sym_scope_delete(struct sym_scope * scope);

struct symbol    * sym_new(struct sym_scope * scope, char * name, enum scalar_type type);
enum scalar_type   sym_type(struct symbol * sym);

#endif
// SYMTABLE_H
