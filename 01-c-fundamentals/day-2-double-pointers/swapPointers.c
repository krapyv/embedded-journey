#include <stdio.h>
#include <stdint.h>

// Swap two pointers (not the values they point to, the pointers themselves)

void swap_pointers(int **a, int **b)
{
    int *tmp = *a; // one star (one dereference) gives us the adress of the pointers to which the double pointer points to
    *a = *b;
    *b = tmp;
}

int main(void)
{
    int x = 5, y = 10;
    int *p_x = &x;
    int *p_y = &y;

    // still have some problems with that formats in printf xD

    // printf("before: p_x = %p, *p_x = %p\n", p_x, *p_x);
    // printf("before: p_y = %d, *p_y = %p\n\n", p_y, *p_y);
    //// we need to pass the pointers by address (reference) to the function
    // swap_pointers(&p_x, &p_y);
    // printf("after: p_x = %d, *p_x = %p\n", p_x, *p_x);
    // printf("after: p_y = %d, *p_y = %p\n\n", p_y, *p_y);

    printf("before: p_x = %p, *p_x = %d\n", (void *)p_x, *p_x);
    printf("before: p_y = %p, *p_y = %d\n\n", (void *)p_y, *p_y);
    // we need to pass the pointers by address (reference) to the function
    swap_pointers(&p_x, &p_y);
    printf("after: p_x = %p, *p_x = %d\n", (void *)p_x, *p_x);
    printf("after: p_y = %p, *p_y = %d\n\n", (void *)p_y, *p_y);
}