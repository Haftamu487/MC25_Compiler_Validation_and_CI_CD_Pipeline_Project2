#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

void init_codegen(void);
void append_code(const char *fmt, ...);

void generate_return(int value);
void generate_assignment(const char *name, int value);
void generate_variable(const char *name);
void generate_return_variable(const char *name);

/* Generate MIPS code for the complete program AST */
void generate_program(ASTNode *node);

/* Generate MIPS code for an AST return expression */
void generate_return_expression(ASTNode *node);

void write_output(const char *filename);
void free_codegen(void);

#endif