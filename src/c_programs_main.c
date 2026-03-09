#include "c_programs.h"
#include <stdio.h>

int main(void) {

    // Compute bitwise add function:
    printf("The bitwise add of %d and %d is: %d\n", 3, 2, bitwiseadd(3,2));

    // Compute the fibbonacci terms:
    int fib[50];
    fibonacci(20, fib);

    // End:
    return 0;
}
