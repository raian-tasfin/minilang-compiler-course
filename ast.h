#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef AST_H
#define AST_H 1

/**
 * Node Types
 */
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_ASSIGN,
    NODE_DECLARE,
    NODE_DECLARE_ASSIGN,
    NODE_PRINT,
} NodeType;

/**
 * Single Node Definition
 */
typedef struct ASTNode ASTNode;
typedef struct ASTNode {
    NodeType type;
    int num;
    char *str;
    ASTNode * left;
    ASTNode * right;
    ASTNode * next;
} ASTNode;

/**
 * Root Definition
 */
extern ASTNode *ast_root;

/**
 * Functions
 */
ASTNode * ast_number(int num);
ASTNode * ast_identifier(const char *str);
ASTNode * ast_binop(NodeType op, ASTNode * left, ASTNode * right);
ASTNode * ast_assign(const char *str, ASTNode *expr);
ASTNode * ast_declare(const char * str);
ASTNode * ast_declare_assign(const char * name, ASTNode * expr);
ASTNode * ast_print(ASTNode * expr);
void      ast_free(ASTNode * node);
void      ast_print_tree(ASTNode * node, int depth);

#endif
// AST_H
