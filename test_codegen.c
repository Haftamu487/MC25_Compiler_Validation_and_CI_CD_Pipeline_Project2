#include <stdio.h>
#include "codegen.h"

int main(void)
{
    init_codegen();

    generate_return(42);

    write_output("output.s");

    free_codegen();

    printf("MIPS code generated successfully in output.s\n");

    return 0;
}
