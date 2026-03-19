#include "c_programs.h"
#include "stdio.h"

int main(void) {

    // 1. Integers multiplication without sign:
    printf("\n\n1. Compute multiplication without sing:\n\n");
    multiplication_without_sign(0x0B, 0x0D);

    // 2. Two complement multiplication:
    printf("\n\n2. Compute multiplication with sing:\n\n");
    multiplication_with_sign(0x0D, 0x05);

    // 3. Calculate power:
    printf("\n\n3. Compute power with sing:\n\n");
    power_with_sign(0x02, 0x03); // 2^3 = 8
    power_with_sign(0x0E, 0x03); // (-2)^3 = -8

    // End:
    return 0;
}
