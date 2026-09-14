#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "codegen.h"

/* ---------------------------------------------------------
   Data section buffer
   --------------------------------------------------------- */

static char *data_buffer = NULL;
static size_t data_size = 0;
static size_t data_capacity = 1024;

/* ---------------------------------------------------------
   Text section buffer
   --------------------------------------------------------- */

static char *code_buffer = NULL;
static size_t code_size = 0;
static size_t code_capacity = 1024;

static int text_started = 0;


/* ---------------------------------------------------------
   Initialize code generator
   --------------------------------------------------------- */

void init_codegen(void)
{
    data_buffer = malloc(data_capacity);
    code_buffer = malloc(code_capacity);

    if (data_buffer == NULL || code_buffer == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");

        free(data_buffer);
        free(code_buffer);

        exit(EXIT_FAILURE);
    }

    data_buffer[0] = '\0';
    code_buffer[0] = '\0';

    data_size = 0;
    code_size = 0;

    text_started = 0;
}


/* ---------------------------------------------------------
   Append to TEXT section
   --------------------------------------------------------- */

void append_code(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0)
    {
        fprintf(stderr, "Error generating code.\n");
        exit(EXIT_FAILURE);
    }

    if (code_size + len + 1 > code_capacity)
    {
        while (code_size + len + 1 > code_capacity)
        {
            code_capacity *= 2;
        }

        char *temp = realloc(code_buffer, code_capacity);

        if (temp == NULL)
        {
            fprintf(stderr, "Error: Memory reallocation failed.\n");
            free(code_buffer);
            exit(EXIT_FAILURE);
        }

        code_buffer = temp;
    }

    va_start(args, fmt);

    vsnprintf(
        code_buffer + code_size,
        len + 1,
        fmt,
        args
    );

    va_end(args);

    code_size += len;
}


/* ---------------------------------------------------------
   Append to DATA section
   --------------------------------------------------------- */

static void append_data(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (len < 0)
    {
        fprintf(stderr, "Error generating data code.\n");
        exit(EXIT_FAILURE);
    }

    if (data_size + len + 1 > data_capacity)
    {
        while (data_size + len + 1 > data_capacity)
        {
            data_capacity *= 2;
        }

        char *temp = realloc(data_buffer, data_capacity);

        if (temp == NULL)
        {
            fprintf(stderr, "Error: Memory reallocation failed.\n");
            free(data_buffer);
            exit(EXIT_FAILURE);
        }

        data_buffer = temp;
    }

    va_start(args, fmt);

    vsnprintf(
        data_buffer + data_size,
        len + 1,
        fmt,
        args
    );

    va_end(args);

    data_size += len;
}


/* ---------------------------------------------------------
   Start TEXT section
   --------------------------------------------------------- */

static void start_text_section(void)
{
    if (!text_started)
    {
        append_code(".text\n");
        append_code(".globl main\n\n");
        append_code("main:\n");

        text_started = 1;
    }
}


/* ---------------------------------------------------------
   Generate variable declaration
   --------------------------------------------------------- */

void generate_variable(const char *name)
{
    append_data("%s: .word 0\n", name);
}


/* ---------------------------------------------------------
   Generate return integer
   --------------------------------------------------------- */

void generate_return(int value)
{
    start_text_section();

    append_code("    li $a0, %d\n", value);

    /* MARS syscall 1: print integer */
    append_code("    li $v0, 1\n");
    append_code("    syscall\n");

    /* MARS syscall 10: exit */
    append_code("    li $v0, 10\n");
    append_code("    syscall\n");
}


/* ---------------------------------------------------------
   Generate return variable
   --------------------------------------------------------- */

void generate_return_variable(const char *name)
{
    start_text_section();

    append_code("    lw $a0, %s\n", name);

    /* MARS syscall 1: print integer */
    append_code("    li $v0, 1\n");
    append_code("    syscall\n");

    /* MARS syscall 10: exit */
    append_code("    li $v0, 10\n");
    append_code("    syscall\n");
}


/* ---------------------------------------------------------
   Generate expression
   --------------------------------------------------------- */

static void generate_expression(ASTNode *node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
        case NODE_NUMBER:

            append_code("    li $t0, %d\n", node->value);

            break;


        case NODE_VARIABLE:

            append_code("    lw $t0, %s\n", node->name);

            break;


        case NODE_ADD:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            append_code("    add $t0, $t1, $t0\n");

            break;


        case NODE_SUB:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            append_code("    sub $t0, $t1, $t0\n");

            break;


        case NODE_MUL:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            append_code("    mul $t0, $t1, $t0\n");

            break;


        case NODE_DIV:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            append_code("    div $t1, $t0\n");
            append_code("    mflo $t0\n");

            break;


        case NODE_LT:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            /* $t0 = ($t1 < $t0) */
            append_code("    slt $t0, $t1, $t0\n");

            break;


        case NODE_GT:

            generate_expression(node->left);

            append_code("    addiu $sp, $sp, -8\n");
            append_code("    sw $t0, 4($sp)\n");

            generate_expression(node->right);

            append_code("    lw $t1, 4($sp)\n");
            append_code("    addiu $sp, $sp, 8\n");

            /* $t0 = ($t1 > $t0) */
            append_code("    slt $t0, $t0, $t1\n");

            break;


        default:

            break;
    }
}


/* ---------------------------------------------------------
   Labels
   --------------------------------------------------------- */

static int label_count = 0;


static int new_label(void)
{
    return label_count++;
}


/* ---------------------------------------------------------
   Generate statement
   --------------------------------------------------------- */

static void generate_statement(ASTNode *node)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
        case NODE_STATEMENT_LIST:

            generate_statement(node->left);
            generate_statement(node->right);

            break;


        case NODE_ASSIGN:

            generate_expression(node->right);

            if (node->left != NULL &&
                node->left->type == NODE_VARIABLE)
            {
                append_code(
                    "    sw $t0, %s\n",
                    node->left->name
                );
            }

            break;


        case NODE_RETURN:

            generate_expression(node->left);

            append_code("    move $a0, $t0\n");

            /* MARS syscall 1: print integer */
            append_code("    li $v0, 1\n");
            append_code("    syscall\n");

            /* MARS syscall 10: exit */
            append_code("    li $v0, 10\n");
            append_code("    syscall\n");

            break;


        case NODE_IF:
        {
            int else_label = new_label();
            int end_label = new_label();

            generate_expression(node->left);

            append_code(
                "    beq $t0, $zero, else_%d\n",
                else_label
            );

            generate_statement(node->right);

            if (node->third != NULL)
            {
                append_code(
                    "    j endif_%d\n",
                    end_label
                );

                append_code(
                    "else_%d:\n",
                    else_label
                );

                generate_statement(node->third);

                append_code(
                    "endif_%d:\n",
                    end_label
                );
            }
            else
            {
                append_code(
                    "else_%d:\n",
                    else_label
                );
            }

            break;
        }


        case NODE_WHILE:
        {
            int start_label = new_label();
            int end_label = new_label();

            append_code(
                "while_%d:\n",
                start_label
            );

            generate_expression(node->left);

            append_code(
                "    beq $t0, $zero, endwhile_%d\n",
                end_label
            );

            generate_statement(node->right);

            append_code(
                "    j while_%d\n",
                start_label
            );

            append_code(
                "endwhile_%d:\n",
                end_label
            );

            break;
        }


        default:

            generate_expression(node);

            break;
    }
}


/* ---------------------------------------------------------
   Generate complete program
   --------------------------------------------------------- */

void generate_program(ASTNode *node)
{
    start_text_section();

    generate_statement(node);
}


/* ---------------------------------------------------------
   Generate return expression
   --------------------------------------------------------- */

void generate_return_expression(ASTNode *node)
{
    start_text_section();

    generate_expression(node);

    append_code("    move $a0, $t0\n");

    /* MARS syscall 1: print integer */
    append_code("    li $v0, 1\n");
    append_code("    syscall\n");

    /* MARS syscall 10: exit */
    append_code("    li $v0, 10\n");
    append_code("    syscall\n");
}


/* ---------------------------------------------------------
   Generate variable assignment
   --------------------------------------------------------- */

void generate_assignment(const char *name, int value)
{
    start_text_section();

    append_code(
        "    li $t0, %d\n",
        value
    );

    append_code(
        "    sw $t0, %s\n",
        name
    );
}


/* ---------------------------------------------------------
   Write complete MIPS output
   --------------------------------------------------------- */

void write_output(const char *filename)
{
    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        fprintf(stderr, "Error: Cannot create output file.\n");
        return;
    }

    /* DATA section */
    fprintf(file, ".data\n");
    fprintf(file, "%s", data_buffer);

    /* TEXT section */
    fprintf(file, "\n");
    fprintf(file, "%s", code_buffer);

    fclose(file);
}


/* ---------------------------------------------------------
   Free code generator
   --------------------------------------------------------- */

void free_codegen(void)
{
    free(data_buffer);
    free(code_buffer);

    data_buffer = NULL;
    code_buffer = NULL;

    data_size = 0;
    code_size = 0;

    text_started = 0;
}