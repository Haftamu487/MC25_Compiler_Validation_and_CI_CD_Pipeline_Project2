#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,

    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,

    NODE_LT,
    NODE_GT,

    NODE_IF,
    NODE_WHILE,
    NODE_ASSIGN,
    NODE_RETURN,

    NODE_STATEMENT_LIST
} NodeType;

typedef struct ASTNode {
    NodeType type;

    int value;
    char *name;

    struct ASTNode *left;
    struct ASTNode *right;

    /* Used for IF/ELSE */
    struct ASTNode *third;
} ASTNode;

/* Create AST nodes */
ASTNode *create_number_node(int value);
ASTNode *create_variable_node(char *name);

ASTNode *create_binary_node(
    NodeType type,
    ASTNode *left,
    ASTNode *right
);

ASTNode *create_if_node(
    ASTNode *condition,
    ASTNode *then_branch,
    ASTNode *else_branch
);

ASTNode *create_while_node(
    ASTNode *condition,
    ASTNode *body
);

ASTNode *create_statement_list(
    ASTNode *statement,
    ASTNode *next
);
ASTNode *create_return_node(ASTNode *expression);

/* Free AST memory */
void free_ast(ASTNode *node);

#endif