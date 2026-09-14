#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"


ASTNode *create_number_node(int value)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_NUMBER;
    node->value = value;
    node->name = NULL;

    node->left = NULL;
    node->right = NULL;
    node->third = NULL;

    return node;
}


ASTNode *create_variable_node(char *name)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_VARIABLE;

    node->name = strdup(name);

    node->left = NULL;
    node->right = NULL;
    node->third = NULL;

    return node;
}


ASTNode *create_binary_node(
    NodeType type,
    ASTNode *left,
    ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = type;

    node->left = left;
    node->right = right;
    node->third = NULL;

    node->name = NULL;
    node->value = 0;

    return node;
}

ASTNode *create_if_node(
    ASTNode *condition,
    ASTNode *then_branch,
    ASTNode *else_branch)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_IF;
    node->value = 0;
    node->name = NULL;

    node->left = condition;
    node->right = then_branch;
    node->third = else_branch;

    return node;
}


ASTNode *create_while_node(
    ASTNode *condition,
    ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_WHILE;
    node->value = 0;
    node->name = NULL;

    node->left = condition;
    node->right = body;
    node->third = NULL;

    return node;
}


ASTNode *create_statement_list(
    ASTNode *statement,
    ASTNode *next)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_STATEMENT_LIST;
    node->value = 0;
    node->name = NULL;

    node->left = statement;
    node->right = next;
    node->third = NULL;

    return node;
}

ASTNode *create_return_node(ASTNode *expression)
{
    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    node->type = NODE_RETURN;
    node->value = 0;
    node->name = NULL;

    node->left = expression;
    node->right = NULL;
    node->third = NULL;

    return node;
}

void free_ast(ASTNode *node)
{
    if (node == NULL)
        return;

    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->third);

    free(node->name);
    free(node);
}
