#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOLS 100

typedef struct {
    char *name;
    char *type;
} Symbol;

void init_symbol_table(void);
int add_symbol(const char *name, const char *type);
Symbol *lookup_symbol(const char *name);
void free_symbol_table(void);
void print_symbol_table(void);

#endif
