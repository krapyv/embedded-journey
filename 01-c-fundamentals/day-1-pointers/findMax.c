#include <stdio.h>
#include <stdint.h>

// Write a function that returns the maximum value
// in an array using pointer arithmetic.

int find_max(const int *arr, int length)
{
    const int *start = arr;
    const int *end = arr + length;

    int max = *start;

    for (; start < end; start++)
    {
        if (*start > max)
        {
            max = *start;
        }
    }

    return max;
}

int main(void)
{
    int arr[10] = {5, 33, 55, 222, 10004, 10, -1, 10, 5440, 0};
    int length = 10;

    int max = find_max(arr, length);

    printf("the max: %d\n", max);
    return 0;
}