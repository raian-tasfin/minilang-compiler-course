#include "ast_kind.h"
#include "../parser/parser.tab.h"


enum ast_kind
astk_from_tok(int token_type)
{
    switch (token_type) {
    case ADD: return AST_ADD;
    case SUB: return AST_SUB;
    case MUL: return AST_MUL;
    case DIV: return AST_DIV;
    case MOD: return AST_MOD;
    case PRNT: return AST_PRNT;
    case INTEGER: return AST_INTEGER;
    default: {
        fprintf(stderr,
                "[AST]: Unexpected token type: %d\n",
                token_type);
        return AST_ERR;
    }
    }
}

char *
astk_tokstr(enum ast_kind type)
{
    switch (type) {
    case AST_ADD: return "AST_ADD";
    case AST_SUB: return "AST_SUB";
    case AST_MUL: return "AST_MUL";
    case AST_DIV: return "AST_DIV";
    case AST_MOD: return "AST_MOD";
    case AST_PRNT: return "AST_PRNT";
    case AST_ERR: return "AST_ERR";
    case AST_INTEGER: return "AST_INTEGER";
    };
}
