#ifndef SYMTABLE_H
#define SYMTABLE_H 1


enum sym_scalar_type {
    SYM_INTEGER,
    SYM_BOOLEAN,
};


struct symbol {
    char * name;
    enum sym_scalar_type type;
    int id;
};

struct sym_scope;

struct sym_scope *   sym_scope_new(struct sym_scope * parent_scope);
struct symbol    *   sym_scope_find_local(struct sym_scope * scope, char * name);
struct symbol    *   sym_scope_find(struct sym_scope * scope, char * name);
struct symbol    *   sym_new(struct sym_scope * scope, char * name, enum sym_scalar_type type);
enum sym_scalar_type sym_type(struct symbol * sym);
void sym_scope_delete(struct sym_scope * scope);
struct sym_scope * sym_scope_child(struct sym_scope * scope, int n);
char * sym_scalar_type_to_str(enum sym_scalar_type type);

#endif
// SYMTABLE_H
