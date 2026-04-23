#include <stdio.h>
#include <stdint.h>

// Version 1: It works, but i'd like not to make temp as just static value
// I want to give the temp var the same address that e.g. the b var has

// void swap(int *a, int *b) {
//     int temp = *b;
//     b = a;
//     a = &temp;

//     printf("the result: a = %d and b = %d\n", *a, *b);
// }

// Version 2: Works, but it does not work as intended. 
// It swaps local copies only, the main variables remain unchanged

// void swap(int *a, int *b) {
//     int *temp = b;
//     b = a;
//     a = temp;

//     printf("the result: a = %d and b = %d\n", *a, *b);
// }

// Version 3 : Final correct version.
// As I understood, the first version did not work as I wanted, because temp is local variable with its own address on the stack, so I cannot make two variables that are not the same to point (share) to the same address
// For swap not only the values between the pointers, but as I wanted, to swap addresses, I need to use pointer to pointer, not just pointer to int
void swap(int *a, int *b) {
    int temp = *b;
    *b = *a;
    *a = temp;

printf("after swap: a = %d (%p) and b = %d (%p)\n", *a, (void*)a, *b, (void*)b);
}


int main() {
    int x = 5;
    int y = 10;
    int *a = &x;
    int *b = &y;

printf("before: a = %d (%p) and b = %d (%p)\n", *a, (void*)a, *b, (void*)b);
    swap(a, b);

printf("after: a = %d (%p) and b = %d (%p)\n", *a, (void*)a, *b, (void*)b);
    return 0;
}