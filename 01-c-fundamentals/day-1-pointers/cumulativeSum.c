#include <stdio.h>
#include <stdint.h>

// Write a function that replaces
// each element with the sum of all previous elements (including itself).

void cumulative_sum(int *arr, int length)
{
    int *prev = arr;

    // we just need to sum the prev element with the current one
    for (int *p = arr + 1; p < arr + length; p++, prev++)
    {
        *p += *prev;
    }
}

int main(void)
{
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int length = 10;

    cumulative_sum(arr, length);

    printf("after modifications:\n");
    for (int i = 0; i < length; i++)
    {
        printf("%d\n", arr[i]);
    }

    return 0;
}