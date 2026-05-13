#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H 1

struct symrec {
    char * name;
    struct symrec * next;
    int type;
};

struct scope {
    struct symrec * head;
    struct scope * parent;
};



struct scope * scope_enter_new(struct scope * curr);
struct scope * scope_exit(struct scope * curr);
struct symrec * symbol_insert(struct scope * scope, char * name, int type);
struct symrec * symbol_lookup(struct scope * scope, char * name);

char * symbol_name(struct symrec * sym);

#endif
// SYMBOL_TABLE_H
