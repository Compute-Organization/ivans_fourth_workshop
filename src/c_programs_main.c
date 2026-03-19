#include "c_programs.h"
#include <stdio.h>



int main(void) {

    // 1. Integers multiplication without sign:
    multiplication_without_sign(0x0B, 0x0D);

    // 2. Two complement multiplication:
    multiplication_with_sign(0x0B, 0x0D);

    // End:
    return 0;
}
