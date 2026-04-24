#include <stdio.h>
#include <stdint.h>

// Write a function that squares every element in-place using pointer arithmetic.

void square_array(int *arr, int length)
{
    for (int *p = arr; p < arr + length; p++)
    {
        *p *= *p;
    }
}

int main(void)
{
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int length = 10;

    square_array(arr, length);

    printf("after modifications:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%d\n", arr[i]);
    }
    return 0;
}