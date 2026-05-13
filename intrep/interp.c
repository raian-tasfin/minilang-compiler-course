#include "interp.h"
#include <stdio.h>


/*********************
 * Private Utilities *
 *********************/
char
ir_optoch(int token_type)
{
    switch (token_type) {
    case ADD: return '+';
    case SUB: return '-';
    case MUL: return '*';
    case DIV: return '/';
    case MOD: return '%';
    default: return '?';
    }
}

struct symrec *
ir_insert_symbol(int * tmpid, struct scope * scope, int type)
{
    if (!tmpid || !scope) return NULL;
    char name[32];
    struct symrec * sym;
    while (1) {
        snprintf(name, sizeof(name), "tmp%d", *tmpid);
        tmpid[0]++;
        if ((sym = symbol_insert(scope, name, type)) != NULL) break;
    }
    tmpid[0]++;
    return sym;
}


struct ir_line
ir_line_new(int type,
            struct symrec * dest,
            union arg arg1,
            union arg arg2);



static struct symrec *
ir_create_rec(struct ast_node * node,
              struct scope * scope,
              struct ir_line * lines,
              int * tmp_id,
              int * line_indx)
{
    if (!node) return NULL;
    switch (node->token_type) {
    case INTEGER: {
        struct symrec * sym = ir_insert_symbol(tmp_id, scope, INTEGER);
        union arg arg1 = { .value = node->value.INTEGER };
        union arg arg2 = { .sym = NULL };
        lines[line_indx[0]++] = ir_line_new(INTEGER, sym, arg1, arg2);
        return sym;
    }
    case AST_SUBEXPR: {
        struct symrec * child_sym =
            ir_create_rec(node->child,
                          scope,
                          lines,
                          tmp_id,
                          line_indx);
        struct symrec * sym = ir_insert_symbol(tmp_id, scope, node->token_type);
        union arg arg1 = { .sym = child_sym };
        union arg arg2 = { .sym = NULL };
        lines[line_indx[0]++] = ir_line_new(AST_SUBEXPR, sym, arg1, arg2);
        return sym;
    }
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD: {
        struct symrec * left_sym =
            ir_create_rec(node->left,
                          scope,
                          lines,
                          tmp_id,
                          line_indx);
        struct symrec * right_sym =
            ir_create_rec(node->right,
                          scope,
                          lines,
                          tmp_id,
                          line_indx);
        struct symrec * sym = ir_insert_symbol(tmp_id, scope, node->token_type);
        union arg arg1 = { .sym = left_sym };
        union arg arg2 = { .sym = right_sym };
        lines[line_indx[0]++] = ir_line_new(node->token_type, sym, arg1, arg2);
        return sym;
    }
    }
    return NULL;
}


void
ir_print_line(struct ir_line * line)
{
    if (!line) return;
    switch (line->type) {
    case INTEGER: {
        fprintf(stdout, "%5s = %10d\n", line->dest->name, line->arg1.value);
        return;
    }
    case AST_SUBEXPR: {
        fprintf(stdout, "%5s = %10s\n",
                line->dest->name,
                line->arg1.sym->name);
        return;
    }
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case MOD: {
        char left[11], right[11];
        snprintf(left, sizeof(left), "%s", line->arg1.sym->name);
        snprintf(right, sizeof(right), "%s", line->arg2.sym->name);
        fprintf(stdout, "%5s = %10s %c %10s\n",
                line->dest->name,
                left,
                ir_optoch(line->type),
                right);
        return;
    }
    }
}


/**************
 * Public-Api *
 **************/
struct ir_line
ir_line_new(int type,
            struct symrec * dest,
            union arg arg1,
            union arg arg2)
{
    return (struct ir_line){
        .type = type,
        .dest = dest,
        .arg1 = arg1,
        .arg2 = arg2
    };
}


struct ir_program *
ir_generate(struct ast_node * root, struct scope * scope)
{
    if (!root) return NULL;
    struct ir_program * prog = malloc(sizeof(struct ir_program));
    int cnt_nodes = ast_cnt_nodes(root);
    prog->size = cnt_nodes;
    prog->lines = malloc(cnt_nodes * sizeof(struct ir_line));
    int tmpid =  0;
    int line_indx =  0;
    ir_create_rec(root, scope, prog->lines, &tmpid, &line_indx);
    return prog;
}

void
ir_print(struct ir_program * prog)
{
    for (int i = 0; i < prog->size; i++) {
        ir_print_line(&prog->lines[i]);
    }
}
