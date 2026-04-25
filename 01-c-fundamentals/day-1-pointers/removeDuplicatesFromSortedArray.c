#include <stdio.h>
#include <stdint.h>

// Write a function that removes consecutive duplicates from a SORTED
// integer array. Return the new length.

int remove_duplicates_sorted(int *arr, int length)
{
    if (length == 0)
        return 0;
    if (length == 1)
        return 1;

    int *write = arr;
    // int *read = arr; // or int *read = arr + 1;
    int *read = arr + 1;

    int new_length = 1;

    while (read < arr + length)
    {
        if (*write != *read)
        {
            write++;
            *write = *read;
            new_length++;
        }
        read++;
    }

    return new_length;
}

int main(void)
{
    int arr[] = {1, 1, 1, 2, 3, 4, 4, 5, 5, 5, 5, 6, 7, 8, 8, 9, 9, 9, 9, 9, 9, 10};
    int length = 22; // after removing duplicates the length should be 10

    int new_length = remove_duplicates_sorted(arr, length);

    printf("new length: %d\n", new_length);
    return 0;
}