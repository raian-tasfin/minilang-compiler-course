#include "ast_kind.h"
#include "../parser/parser.tab.h"


enum ast_binop_type
astk_binop_from_tok(int token_type)
{
    switch (token_type) {
    case ADD: return AST_ADD;
    case SUB: return AST_SUB;
    case MUL: return AST_MUL;
    case DIV: return AST_DIV;
    case MOD: return AST_MOD;
    case AND: return AST_AND;
    case OR: return AST_OR;
    case XOR: return AST_XOR;
    }
}


char *
astk_kind_to_str(enum ast_kind kind)
{
    switch (kind) {
    case AST_INTEGER:    return "AST_INTEGER";
    case AST_BOOLEAN:    return "AST_BOOLEAN";
    case AST_BINOP:      return "AST_BINOP";
    case AST_PRNT:       return "AST_PRNT";
    case AST_BLOCK:      return "AST_BLOCK";
    case AST_PUNCTUATOR: return "AST_PUNCTUATOR";
    }
}

char *
astk_punc_to_str(enum ast_punctuator_type type)
{
    switch (type) {
    case AST_ERR: return "AST_ERR";
    }
}

char *
astk_binop_to_str(enum ast_binop_type type)
{
    switch (type) {
    case AST_ADD: return "AST_ADD";
    case AST_SUB: return "AST_SUB";
    case AST_MUL: return "AST_MUL";
    case AST_DIV: return "AST_DIV";
    case AST_MOD: return "AST_MOD";
    case AST_AND: return "AST_AND";
    case AST_OR:  return "AST_OR";
    case AST_XOR: return "AST_XOR";
    }
}
