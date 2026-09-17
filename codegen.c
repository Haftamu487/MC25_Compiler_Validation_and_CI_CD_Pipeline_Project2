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
   Peephole optimization
   --------------------------------------------------------- */

static void peephole_optimize(void)
{
    char *optimized_buffer;
    size_t optimized_size = 0;
    size_t optimized_capacity = 1024;

    optimized_buffer = malloc(optimized_capacity);

    if (optimized_buffer == NULL)
    {
        fprintf(stderr,
                "Error: Peephole optimization memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    optimized_buffer[0] = '\0';

    char previous_line[256] = "";

    char *current = code_buffer;

    while (*current != '\0')
    {
        char *line_end = strchr(current, '\n');

        size_t line_length;

        if (line_end != NULL)
            line_length = (size_t)(line_end - current) + 1;
        else
            line_length = strlen(current);

        char line[256];

        if (line_length >= sizeof(line))
        {
            fprintf(stderr,
                    "Error: MIPS instruction line is too long.\n");
            free(optimized_buffer);
            exit(EXIT_FAILURE);
        }

        memcpy(line, current, line_length);
        line[line_length] = '\0';

        /*
         * Optimization:
         *
         *     sw $t0, variable
         *     lw $t0, variable
         *
         * The load is redundant because $t0 already
         * contains the value that was just stored.
         */

        if (strcmp(line, "    add $t0, $t0, $zero\n") == 0)
        {
            current += line_length;
            continue;
        } 

        if (strncmp(line, "    lw $t0, ", 12) == 0)
        {
            char variable[128];

            if (sscanf(line,
                       "    lw $t0, %127s",
                       variable) == 1)
            {
                char expected_store[256];

                snprintf(expected_store,
                         sizeof(expected_store),
                         "    sw $t0, %s\n",
                         variable);

                if (strcmp(previous_line, expected_store) == 0)
                {
                    current += line_length;
                    continue;
                }
            }
        }

        if (optimized_size + line_length + 1 > optimized_capacity)
        {
            while (optimized_size + line_length + 1 > optimized_capacity)
                optimized_capacity *= 2;

            char *temp = realloc(optimized_buffer,
                                 optimized_capacity);

            if (temp == NULL)
            {
                fprintf(stderr,
                        "Error: Peephole optimization reallocation failed.\n");

                free(optimized_buffer);
                exit(EXIT_FAILURE);
            }

            optimized_buffer = temp;
        }

        memcpy(optimized_buffer + optimized_size,
               line,
               line_length);

        optimized_size += line_length;
        optimized_buffer[optimized_size] = '\0';

        strncpy(previous_line,
                line,
                sizeof(previous_line) - 1);

        previous_line[sizeof(previous_line) - 1] = '\0';

        current += line_length;
    }

    free(code_buffer);

    code_buffer = optimized_buffer;
    code_size = optimized_size;
    code_capacity = optimized_capacity;
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

            append_code("    add $t0, $t0, $zero\n");
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

    peephole_optimize();

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