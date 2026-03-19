#include "c_programs.h"
#include <stdio.h>

int bitwiseadd(int x, int y){
    while(y != 0){
        int carry = x & y;
        x = x ^ y;
        y = carry << 1;
    }
    return x;
}

void fibonacci(int n, int *fib){
    int t1 = 0;
    int t2 = 1;
    int nextTerm = 0;
    if ( fib == NULL || n <= 0){
        return;
    }
    for ( int i = 0; i < n; ++i ){
        fib[i] = t1;
        printf("fib[%d] = %d\n", i, t1);
        nextTerm = t1 + t2;
        t1 = t2;
        t2 = nextTerm;
    }
}

/* Print 4 bits */
void print_bin4(unsigned char x) {
    for (int i = 3; i >= 0; --i) {
        printf("%d", (x >> i) & 0x01);
    }
}

/* Print 8 bits */
void print_bin8(unsigned char x) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (x >> i) & 0x01);
    }
}

/* Interpret a 4-bit value as signed two's complement */
int sign_extend_4(unsigned char x) {
    x &= 0x0F;
    if (x & 0x08) {
        return (int)(x | 0xF0);
    }
    return (int)x;
}

void multiplication_without_sign(unsigned char M, unsigned char Q) {
    
    // 1. First we define the initial conditions:
    unsigned char A = 0x00;             // 0000
    unsigned char count = 0x04;         // n - 4: 0100
    unsigned char c = 0x00;             // 0000

    // Print in terminal:
    printf("We have the initial conditions:\n");
    printf("M = "); print_bin4(M); printf("\n");
    printf("Q = "); print_bin4(Q); printf("\n");
    printf("A = "); print_bin4(A); printf("\n");
    printf("count = "); print_bin4(count); printf("\n");
    printf("c = %d\n", c); printf("\n");

    while (count > 0x00){

        // 2. Evaluate the condition:
        if(Q & 0x01){

            // 2.1. Sum accumulator (A) with multiplicand (M):
            A = A + M;
            printf("A + M: A = "); print_bin4(A); printf(" (%d)\n", A);
        }

        // 3. Shift right C, A, Q and count -= 1: 
        unsigned short combined = (c << 8) | (A << 4) | Q;  // Combine C, A, Q
        combined = combined >> 1;
        print_bin4(combined);
        
        // We separate again:
        c = (combined >> 8) & 1;    
        A = (combined >> 4) & 0x0F; 
        Q = combined & 0x0F;  
        count = count - 1;
    };

    // 4. Show the result:
    printf("The result is:\n");
    printf("A = "); print_bin4(A); printf("\n");
    printf("Q = "); print_bin4(Q); printf("\n");
    printf("Result = "); print_bin4(A); printf(" "); print_bin4(Q); printf(" - %d\n", (A << 4) | Q); 
}

int multiplication_with_sign(unsigned char M, unsigned char Q) {

    /* Keep only 4 bits */
    M &= 0x0F;
    Q &= 0x0F;

    /* 1. Initial conditions */
    unsigned char A = 0x00;     /* 4 bits */
    unsigned char Q_1 = 0x00;   /* 1 bit */
    unsigned char count = 0x04; /* 4 iterations */

    printf("Initial conditions:\n");
    printf("M   = "); print_bin4(M); printf(" (%d)\n", sign_extend_4(M));
    printf("Q   = "); print_bin4(Q); printf(" (%d)\n", sign_extend_4(Q));
    printf("A   = "); print_bin4(A); printf(" (%d)\n", sign_extend_4(A));
    printf("Q-1 = %d\n", Q_1);
    printf("count = %d\n\n", count);

    while (count > 0x00) {

        /* 2. Booth conditions using only masks + ifs */
        if (((Q & 0x01) == 0x00) && (Q_1 == 0x01)) {
            /* Case 01 -> A = A + M */
            A = (unsigned char)((sign_extend_4(A) + sign_extend_4(M)) & 0x0F);
            printf("Condition (Q0,Q-1) = (0,1) -> A = A + M\n");
        }
        else if (((Q & 0x01) == 0x01) && (Q_1 == 0x00)) {
            /* Case 10 -> A = A - M */
            A = (unsigned char)((sign_extend_4(A) - sign_extend_4(M)) & 0x0F);
            printf("Condition (Q0,Q-1) = (1,0) -> A = A - M\n");
        }
        else if (((Q & 0x01) == 0x00) && (Q_1 == 0x00)) {
            /* Case 00 -> do nothing */
            printf("Condition (Q0,Q-1) = (0,0) -> No operation\n");
        }
        else if (((Q & 0x01) == 0x01) && (Q_1 == 0x01)) {
            /* Case 11 -> do nothing */
            printf("Condition (Q0,Q-1) = (1,1) -> No operation\n");
        }

        printf("Before shift:\n");
        printf("A   = "); print_bin4(A); printf(" (%d)\n", sign_extend_4(A));
        printf("Q   = "); print_bin4(Q); printf(" (%d)\n", sign_extend_4(Q));
        printf("Q-1 = %d\n", Q_1);

        /* 3. Arithmetic right shift of [A Q Q-1] */
        unsigned char old_A0 = A & 0x01;         /* LSB of A goes into MSB of Q */
        unsigned char old_Q0 = Q & 0x01;         /* LSB of Q goes into Q-1 */
        unsigned char sign_A = (A >> 3) & 0x01;  /* Sign bit of A */

        Q_1 = old_Q0;
        Q = ((old_A0 << 3) | (Q >> 1)) & 0x0F;
        A = ((sign_A << 3) | (A >> 1)) & 0x0F;

        count = count - 1;

        printf("After shift:\n");
        printf("A   = "); print_bin4(A); printf(" (%d)\n", sign_extend_4(A));
        printf("Q   = "); print_bin4(Q); printf(" (%d)\n", sign_extend_4(Q));
        printf("Q-1 = %d\n", Q_1);
        printf("count = %d\n\n", count);
    }

    /* 4. Final 8-bit result in [A Q] */
    unsigned char result = (unsigned char)((A << 4) | Q);

    /* Interpret final 8-bit result as signed */
    int decimal_result;
    if (result & 0x80) {
        decimal_result = (int)result - 256;
    } else {
        decimal_result = (int)result;
    }

    printf("Final result:\n");
    printf("A = "); print_bin4(A); printf("\n");
    printf("Q = "); print_bin4(Q); printf("\n");
    printf("AQ = "); print_bin8(result); printf(" (%d)\n", decimal_result);

    return decimal_result;
}
