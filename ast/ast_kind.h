#ifndef AST_KIND_H
#define AST_KIND_H 1


/* Types of nodes with values */
enum ast_kind {
    AST_SCALAR,
    AST_IDENT,
    AST_BINOP,
    AST_UNOP,
    AST_PRNT,
    AST_BLOCK,
    AST_PUNCTUATOR,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_WHILE_LOOP,
    AST_COND,
    AST_IF,
    AST_ELSE,
};


/* Types of nodes that have no values */
enum ast_punctuator_type {
    AST_ERR,
};

/* Binary operators */
enum ast_binop_type {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_AND,
    AST_OR,
    AST_XOR,
    AST_LT,
    AST_LE,
    AST_GT,
    AST_GE,
    AST_NE,
    AST_EQ,
};

/* Unary operators */
enum ast_unop_type {
    AST_NEG,
    AST_NOT,
};

/* Enum to string */
char * astk_kind_to_str(enum ast_kind kind);
char * astk_punc_to_str(enum ast_punctuator_type type);
char * astk_binop_to_str(enum ast_binop_type type);
char * astk_unop_to_str(enum ast_unop_type type);

/* Alien enum to ast enum */
enum ast_binop_type
astk_binop_from_tok(int token_type);

enum ast_unop_type
astk_unop_from_tok(int token_type);

#endif
// AST_KIND_H
