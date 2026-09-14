#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

static Symbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;

void init_symbol_table(void)
{
    symbol_count = 0;
}

int add_symbol(const char *name, const char *type)
{
    if (symbol_count >= MAX_SYMBOLS)
    {
        fprintf(stderr, "Error: Symbol table is full.\n");
        return 0;
    }

    if (lookup_symbol(name) != NULL)
    {
        fprintf(stderr, "Semantic Error: Variable '%s' already declared.\n", name);
        return 0;
    }

    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].type = strdup(type);
    symbol_count++;

    return 1;
}

Symbol *lookup_symbol(const char *name)
{
    for (int i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
        {
            return &symbol_table[i];
        }
    }

    return NULL;
}

void print_symbol_table(void)
{
    printf("\n--- Symbol Table ---\n");

    for (int i = 0; i < symbol_count; i++)
    {
        printf("Name: %s, Type: %s\n",
               symbol_table[i].name,
               symbol_table[i].type);
    }

    printf("--------------------\n");
}

void free_symbol_table(void)
{
    for (int i = 0; i < symbol_count; i++)
    {
        free(symbol_table[i].name);
        free(symbol_table[i].type);
    }

    symbol_count = 0;
}
