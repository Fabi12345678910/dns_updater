#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "helper_functions/common.h"

extern int test_implementation(void);

int main(int argc, char const *argv[])
{
    (void) argv;
    if (argc == 1){
        printf("no arguments provided, testing implementation\n");
        return test_implementation();
    }
    return EXIT_SUCCESS;
}
