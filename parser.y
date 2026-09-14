%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"

#include "symbol_table.h"

int yylex(void);
void yyerror(const char *s);

int semantic_error = 0;
ASTNode *program_ast = NULL;
%}

%union {
    int num;
    char *str;
    ASTNode *node;
}

%token <num> NUMBER
%token <str> ID

%token INT RETURN IF ELSE WHILE

%type <node> expression
%type <node> condition
%type <node> statement
%type <node> statements
%type <node> block

%left '+' '-'
%left '*' '/'

%start program

%%

program:
      INT ID '(' ')' '{' declarations statements '}'
      {
          program_ast = $7;

          printf("Syntax analysis successful!\n");
          printf("Valid MC-25 program detected.\n");
          printf("Function name: %s\n", $2);

          print_symbol_table();

          generate_program(program_ast);

          free($2);
      }
      ;

declarations:
      declarations declaration
    |
      /* empty */
    ;

declaration:
      INT ID ';'
      {
          if (add_symbol($2, "int"))
          {
              printf("Semantic Analysis: Variable '%s' declared as int.\n", $2);

              generate_variable($2);
          }
          else
          {
            semantic_error = 1;
          }

          free($2);
      }

    |
      INT ID '=' NUMBER ';'
      {
          if (add_symbol($2, "int"))
          {
              printf("Semantic Analysis: Variable '%s' declared and initialized to %d.\n",
                     $2, $4);

              generate_variable($2);

              generate_assignment($2, $4);
          }
          else
          {
              semantic_error = 1;
          }

          free($2);
      }
    
    ;

statements:
      /* empty */
      {
          $$ = NULL;
      }
    |
      statement statements
      {
          $$ = create_statement_list($1, $2);
      }
    ;

block:
      '{' statements '}'
      {
          $$ = $2;
      }
    ;

statement:
      ID '=' expression ';'
      {
          Symbol *symbol = lookup_symbol($1);

          if (symbol == NULL)
          {
              fprintf(stderr,
                      "Semantic Error: Variable '%s' not declared.\n",
                      $1);
              semantic_error = 1;

              free_ast($3);
              $$ = NULL;
          }
          else
          {
              printf("Semantic Analysis: Assignment to '%s' is valid.\n",
                     $1);

              ASTNode *variable = create_variable_node($1);
              $$ = create_binary_node(NODE_ASSIGN, variable, $3);
          }

          free($1);
      }

    |
      RETURN expression ';'
      {
          $$ = create_return_node($2);
      }

    |
      IF '(' condition ')' block
      {
          $$ = create_if_node($3, $5, NULL);
      }

    |
      IF '(' condition ')' block ELSE block
      {
          $$ = create_if_node($3, $5, $7);
      }

    |
      WHILE '(' condition ')' block
      {
          $$ = create_while_node($3, $5);
      }
    ;

condition:
      expression '<' expression
      {
          $$ = create_binary_node(NODE_LT, $1, $3);
      }
    |
      expression '>' expression
      {
          $$ = create_binary_node(NODE_GT, $1, $3);
      }
    ;

expression:
      NUMBER
      {
          $$ = create_number_node($1);
      }
    |
      ID
      {
          Symbol *symbol = lookup_symbol($1);

          if (symbol == NULL)
          {
              fprintf(stderr,
                      "Semantic Error: Variable '%s' not declared.\n",
                      $1);
              semantic_error = 1;
              $$ = create_number_node(0);
          }
          else
          {
              printf("Semantic Analysis: Variable '%s' used.\n", $1);
              $$ = create_variable_node($1);
          }

          free($1);
      }
    |
expression '+' expression
{
    $$ = create_binary_node(NODE_ADD, $1, $3);
}
|
expression '-' expression
{
    $$ = create_binary_node(NODE_SUB, $1, $3);
}
|
expression '*' expression
{
    $$ = create_binary_node(NODE_MUL, $1, $3);
}
|
expression '/' expression
{
    $$ = create_binary_node(NODE_DIV, $1, $3);
}

;


%%

void yyerror(const char *s)
{
    fprintf(stderr, "Syntax Error: %s\n", s);
}

int main(void)
{
    printf("Starting MC-25 Compiler...\n");

    init_symbol_table();
    init_codegen();

    if (yyparse() == 0)
    {
        if (semantic_error)
        {
            printf("Compilation failed due to semantic errors.\n");
            free_ast(program_ast);
            free_codegen();
            free_symbol_table();
            return 1;
        }

        printf("Parsing completed successfully.\n");

        write_output("output.s");

        free_ast(program_ast);

        free_codegen();
        free_symbol_table();

        return 0;
    }

    free_codegen();
    free_symbol_table();
    return 1;
}