#include "interp.h"
#include "../ast/ast_kind.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


struct ir_sym *
ir_sym_new(char * name, enum ir_type type, int * var_id)
{
    struct ir_sym * new = malloc(sizeof(struct ir_sym));
    new[0] = (struct ir_sym){
        .id = var_id[0]++,
        .type = type
    };
    if (name) {
        strncpy(new->src_name, name, 63);
        new->src_var = true;
    }
    return new;
}

struct ir_stmt *
ir_stmt_binop_new(enum ir_binop op,
                  struct ir_sym * dst,
                  struct ir_arg arg1,
                  struct ir_arg arg2)
{
    struct ir_stmt * new = malloc(sizeof(struct ir_stmt));
    *new = (struct ir_stmt) {
        .type = IR_BINOP_ASSIGNMENT,
        .binop_asn = {
            .dst = dst,
            .arg1 = arg1,
            .arg2 = arg2,
        },
    };
    return new;
}

void
ir_block_push(struct ir_stmt stmt, struct ir_block * block)
{
    if (block->size == block->cap) {
        block->cap = (5 + block->cap) * 2;
        block->stmts
            = realloc(block->stmts,
                      block->cap * sizeof(struct ir_stmt));
    }
    block->stmts[block->size++] = stmt;
}

enum ir_binop
ir_astk_to_ir_binop(enum ast_kind type)
{
    switch (type) {
    case AST_ADD: return IR_ADD;
    case AST_SUB: return IR_SUB;
    case AST_MUL: return IR_MUL;
    case AST_DIV: return IR_DIV;
    case AST_MOD: return IR_MOD;
    };
}




struct ir_arg
ir_block_generate_rec(struct ast_node * node,
                      int * var_id,
                      struct ir_block * block)
{
    switch (node->type) {
    case AST_INTEGER: {
        struct ir_sym * dest = ir_sym_new(NULL, IR_INTEGER, var_id);
        struct ir_stmt stmt = {
            .type = IR_CONST_ASSIGNMENT,
            .const_asn = {
                .dest = dest,
                .val = node->value.INTEGER,
            }
        };
        ir_block_push(stmt, block);
        return (struct ir_arg) {
            .type = IR_INTEGER,
            .sym = dest,
        };
    }
    case AST_PRNT: {
        struct ir_arg print_arg = ir_block_generate_rec(node->child, var_id, block);
        struct ir_stmt stmt = {
            .type = IR_PRINT,
            .prnt = {
                .arg = print_arg
            }
        };
        ir_block_push(stmt, block);
        return (struct ir_arg){0};
    }
    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD: {
        struct ir_sym * dest = ir_sym_new(NULL, IR_INTEGER, var_id);
        struct ir_arg arg1 = ir_block_generate_rec(node->left, var_id, block);
        struct ir_arg arg2 = ir_block_generate_rec(node->right, var_id, block);
        struct ir_stmt stmt = {
            .type = IR_BINOP_ASSIGNMENT,
            .binop_asn = {
                .op = ir_astk_to_ir_binop(node->type),
                .dst = dest,
                .arg1 = arg1,
                .arg2 = arg2
            }
        };
        ir_block_push(stmt, block);
        return (struct ir_arg) {
            .sym = dest,
            .type = IR_INTEGER,
        };
    }
    }
}

struct ir_block *
ir_block_generate(struct ast_node * root)
{
    if (!root) return NULL;
    int var_id = 0;
    struct ir_block * new = malloc(sizeof(struct ir_block));
    *new = (struct ir_block){0};
    ir_block_generate_rec(root, &var_id, new);
    return new;
}


void
ir_fprintf(struct ir_ctx * ctx, const char *fmt, ...)
{
    if (!ctx || !ctx->rprt) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(ctx->rprt, fmt, args);
    va_end(args);
}


/**************/
/* Public API */
/**************/
struct ir_ctx
ir_ctx_init(struct cli_opts * cliopts)
{
    struct ir_ctx ctx = { 0 };
    if (!cliopts) return ctx;

    /**************
     * Report Ctx *
     **************/
    if (!cliopts->ir.rprt) return ctx;
    if (!cliopts->ir.path) ctx.rprt = stdout;
    else ctx.rprt = fopen(cliopts->ir.path, "w");
    if (!ctx.rprt) {
        fprintf(stderr,
                "Could not open file '%s'\n",
                cliopts->ir.path);
        ctx.err = true;
    }

    return ctx;
}


void
ir_sym_to_str(struct ir_sym * sym, char * dest)
{
    if (sym->src_var) {
        sprintf(dest, "%s", sym->src_name);
        return;
    }
    sprintf(dest, "tmp_%d", sym->id);
}

void
ir_arg_to_str(struct ir_arg arg, char * dest)
{
    ir_sym_to_str(arg.sym, dest);
}

char
ir_opch(enum ir_binop type)
{
    switch (type) {
    case IR_ADD: return '+';
    case IR_SUB: return '-';
    case IR_MUL: return '*';
    case IR_DIV: return '/';
    case IR_MOD: return '%';
    default: return '?';
    }
}

void
ir_print(struct ir_ctx * ctx, struct ir_block * block)
{
    if (!ctx || !ctx->rprt || ctx->err || !block) return;
    for (int i = 0; i < block->size; i++) {
        switch (block->stmts[i].type) {
        case IR_CONST_ASSIGNMENT: {
            char dest[64];
            ir_sym_to_str(block->stmts[i].const_asn.dest, dest);
            int val = block->stmts[i].const_asn.val;
            ir_fprintf(ctx, "%5s = %d\n", dest, val);
            break;
        };
        case IR_BINOP_ASSIGNMENT: {
            switch (block->stmts[i].binop_asn.op) {
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
            case IR_MOD: {
                char dest[64];
                char arg1[64];
                char arg2[64];
                ir_sym_to_str(block->stmts[i].binop_asn.dst, dest);
                ir_arg_to_str(block->stmts[i].binop_asn.arg1, arg1);
                ir_arg_to_str(block->stmts[i].binop_asn.arg2, arg2);
                ir_fprintf(ctx,
                           "%5s = %5s %c %5s\n",
                           dest,
                           arg1,
                           ir_opch(block->stmts[i].binop_asn.op),
                           arg2);
                break;
            }
            }
            break;
        }
        case IR_PRINT: {
            char arg_str[64];
            ir_arg_to_str(block->stmts[i].prnt.arg, arg_str);
            ir_fprintf(ctx, "print %s\n", arg_str);
            break;
        }
        }
    }
}
