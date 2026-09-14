#include <stdio.h>
#include "symbol_table.h"

int main(void)
{
    init_symbol_table();

    add_symbol("x", "int");
    add_symbol("y", "int");

    print_symbol_table();

    Symbol *symbol = lookup_symbol("x");

    if (symbol != NULL)
    {
        printf("Found variable: %s, Type: %s\n",
               symbol->name, symbol->type);
    }
    else
    {
        printf("Variable not found.\n");
    }

    free_symbol_table();

    return 0;
}
