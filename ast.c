#include "ast.h"

ASTNode * ast_root = NULL;

static ASTNode *
new_node(NodeType type)
{
    ASTNode *n = calloc(1, sizeof(ASTNode));
    n-> type = type;
    return n;
}

ASTNode *
ast_number(int num)
{
    ASTNode * n = new_node(NODE_NUMBER);
    n->num = num;
    return n;
}


ASTNode *
ast_identifier(const char *str)
{
    ASTNode * n = new_node(NODE_IDENTIFIER);
    n->str = strdup(str);
    return n;
}

ASTNode *
ast_binop(NodeType op, ASTNode * left, ASTNode * right)
{
    ASTNode * n = new_node(op);
    n->left = left;
    n->right = right;
    return n;
}

ASTNode *
ast_assign(const char *str, ASTNode *expr)
{
    ASTNode * n = new_node(NODE_ASSIGN);
    n->str = strdup(str);
    n->left = expr;
    return n;
}

ASTNode * ast_declare(const char * str)
{
    ASTNode * n = new_node(NODE_DECLARE);
    n->str = strdup(str);
    return n;
}

ASTNode *
ast_declare_assign(const char * str, ASTNode * expr)
{
    ASTNode * n = new_node(NODE_DECLARE_ASSIGN);
    n->str= strdup(str);
    n->left = expr;
    return n;
}

ASTNode *
ast_print(ASTNode * expr)
{
    ASTNode * n =new_node(NODE_PRINT);
    n->left = expr;
    return n;
}

void
ast_free(ASTNode * node)
{
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->next);
    free(node->str);
    free(node);
}

void ast_print_tree(ASTNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->type) {
        case NODE_NUMBER:         printf("[NUMBER]         %d\n", node->num ); break;
        case NODE_IDENTIFIER:     printf("[IDENTIFIER]     %s\n", node->str ); break;
        case NODE_ASSIGN:         printf("[ASSIGN]         %s\n",  node->str); break;
        case NODE_DECLARE:        printf("[DECLARE]        %s\n",  node->str); break;
        case NODE_DECLARE_ASSIGN: printf("[DECLARE_ASSIGN] %s\n", node->str);  break;
        case NODE_ADD:            printf("[ADD]\n"                         );  break;
        case NODE_SUB:            printf("[SUB]\n"                         );  break;
        case NODE_MUL:            printf("[MUL]\n"                         );  break;
        case NODE_DIV:            printf("[DIV]\n"                         );  break;
        case NODE_PRINT:          printf("[PRINT]\n"                       );  break;
        default:                  printf("[?]\n"                           );  break;
    }

    ast_print_tree(node->left,  depth + 1);
    ast_print_tree(node->right, depth + 1);
    ast_print_tree(node->next,  depth);
}
