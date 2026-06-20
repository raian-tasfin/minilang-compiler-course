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
    case OR:  return AST_OR;
    case XOR: return AST_XOR;
    case LT:  return AST_LT;
    case LE:  return AST_LE;
    case GT:  return AST_GT;
    case GE:  return AST_GE;
    case NE:  return AST_NE;
    case EQ:  return AST_EQ;
    }
}


enum ast_unop_type
astk_unop_from_tok(int token_type)
{
    // unary sub is neg
    switch (token_type) {
    case SUB: return AST_NEG;
    case NOT: return AST_NOT;
    }
}



char *
astk_kind_to_str(enum ast_kind kind)
{
    switch (kind) {
    case AST_SCALAR:      return "AST_SCALAR";
    case AST_IDENT:       return "AST_IDENT";
    case AST_BINOP:       return "AST_BINOP";
    case AST_UNOP:        return "AST_UNOP";
    case AST_PRNT:        return "AST_PRNT";
    case AST_BLOCK:       return "AST_BLOCK";
    case AST_PUNCTUATOR:  return "AST_PUNCTUATOR";
    case AST_DECLARATION: return "AST_DECLARATION";
    case AST_ASSIGNMENT:  return "AST_ASSIGNMENT";
    case AST_WHILE_LOOP:  return "AST_WHILE_LOOP";
    case AST_COND:        return "AST_COND";
    case AST_IF:          return "AST_IF";
    case AST_ELIF:        return "AST_ELIF";
    case AST_ELSE:        return "AST_ELSE";
    default: return "UNKNOWN";
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
    case AST_LT:  return "AST_LT";
    case AST_LE:  return "AST_LE";
    case AST_GT:  return "AST_GT";
    case AST_GE:  return "AST_GE";
    case AST_NE:  return "AST_NE";
    case AST_EQ:  return "AST_EQ";

    }
}

char *
astk_unop_to_str(enum ast_unop_type type)
{
    switch (type) {
    case AST_NEG: return "AST_NEG";
    case AST_NOT: return "AST_NOT";
    }
}
