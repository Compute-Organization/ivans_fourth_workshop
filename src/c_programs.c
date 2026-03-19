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

void print_bin(uint8_t num) {
    for (int i = 3; i >= 0; i--) {
        printf("%d", (num >> i) & 1);
    }
}

void multiplication_without_sign(unsigned char M, unsigned char Q) {
    
    // 1. First we define the initial conditions:
    unsigned char A = 0x00;             // 0000
    unsigned char count = 0x04;         // n - 4: 0100
    unsigned char c = 0x00;             // 0000

    // Print in terminal:
    printf("We have the initial conditions:\n");
    printf("M = "); print_bin(M); printf("\n");
    printf("Q = "); print_bin(Q); printf("\n");
    printf("A = "); print_bin(A); printf("\n");
    printf("count = "); print_bin(count); printf("\n");
    printf("c = %d\n", c); printf("\n");

    while (count > 0x00){

        // 2. Evaluate the condition:
        if(Q & 0x01){

            // 2.1. Sum accumulator (A) with multiplicand (M):
            A = A + M;
            printf("A + M: A = "); print_bin(A); printf(" (%d)\n", A);
        }

        // 3. Shift right C, A, Q and count -= 1: 
        unsigned short combined = (c << 8) | (A << 4) | Q;  // Combine C, A, Q
        combined = combined >> 1;
        print_bin(combined);
        
        // We separate again:
        c = (combined >> 8) & 1;    
        A = (combined >> 4) & 0x0F; 
        Q = combined & 0x0F;  

        count = count - 1;
    };

    // 4. Show the result:
    printf("The result is:\n");
    printf("A = "); print_bin(A); printf("\n");
    printf("Q = "); print_bin(Q); printf("\n");
    printf("Result = "); print_bin(A); printf(" "); print_bin(Q); printf(" - %d\n", (A << 4) | Q); 
}

int multiplication_with_sign(unsigned char M, unsigned char Q){

    // 1. First we define the initial conditions:
    unsigned char A = 0x00;             // 0000
    unsigned char Q_1 = 0x00;           // 0000
    unsigned char count = 0x04;         // n - 4: 0100
    unsigned char c = 0x00;             // 0000

}
