#ifndef AST_KIND_H
#define AST_KIND_H 1


/* Types of nodes with values */
enum ast_kind {
    AST_SCALAR,
    AST_BINOP,
    AST_PRNT,
    AST_BLOCK,
    AST_PUNCTUATOR,
};


enum ast_scalar_type {
    AST_BOOLEAN,
    AST_INTEGER,
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
};


/* Enum to string */
char * astk_kind_to_str(enum ast_kind kind);
char * astk_punc_to_str(enum ast_punctuator_type type);
char * astk_binop_to_str(enum ast_binop_type type);
char * astk_scalar_to_str(enum ast_scalar_type type);

/* Alien enum to ast enum */
enum ast_binop_type
astk_binop_from_tok(int token_type);


#endif
// AST_KIND_H
