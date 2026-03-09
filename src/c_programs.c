#include "c_programs.h"
#include <stdio.h>

int bitwiseadd(int x, int y){
    while(y != 0){
        int carry = x & y;
        x = x ^ y;
        y = carry << 1;
    }
    return x;
};


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
