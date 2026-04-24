#include <stdio.h>
#include <stdint.h>

// Write a function that adds a value to every element in an
// array using pointer arithmetic (no index variables).

void increment_all(int *arr, int length, int increment)
{
    // we just need to traverse all the array to its last element and add the increment to its value (not to its address, so we are dereferencing the pointer)
    int *p;
    for (p = arr; p < arr + length; p++)
    {
        *p += increment;
    }
}

int main(void)
{
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int length = 10;
    int increment = 10;

    increment_all(arr, length, increment);

    printf("After incrementation:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%d\n", arr[i]);
    }
    return 0;
}
