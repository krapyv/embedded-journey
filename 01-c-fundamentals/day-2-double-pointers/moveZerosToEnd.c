#include <stdio.h>
#include <stdint.h>

// Write a function that moves all zeros to the end of an array while preserving the order
// of non-zero elements. Use two pointers.

void move_zeros(int *arr, int length)
{
    if (length == 0 || length == 1)
    {
        return;
    }
    if (arr == NULL)
    {
        return;
    }

    int *p_track = arr;
    int *p_current = arr + 1;

    // p_track will help me to keep an eye on the already moved to the beginning non-zero elements and the position that needs to be filled
    while (p_current < arr + length)
    {
        if (*p_track != 0)
        {
            p_track++;
            p_current++;
            continue;
        }
        if (*p_current == 0)
        {
            p_current++;
            continue;
        }
        if (*p_current != 0)
        {
            *p_track = *p_current;
            *p_current = 0;
            p_track++;
            p_current++;
        }
    }
}

// another version (simplier)
void move_zeros1(int *arr, int length)
{
    if (length <= 1 || arr == NULL)
    {
        return;
    }

    int *p_write = arr;
    int *p_read = arr;

    // move all non-zero elements to the beginning
    while (p_read < arr + length)
    {
        if (*p_read != 0)
        {
            *p_write = *p_read;
            p_write++;
        }
        p_read++;
    }

    // fill the rest with zero
    while (p_write < arr + length)
    {
        // well, it is redundant, because all should be filled with 0
        // if (*p_write != 0)
        // {
        *p_write = 0;
        // }
        p_write++;
    }
}

int main(void)
{
    int length = 10;
    int arr[] = {1, 1, 20, 0, 0, 1123, 0, 0, 0, 10};

    move_zeros(arr, length);

    printf("After moving zeros:\n");
    for (int *p = arr; p < arr + length; p++)
    {
        printf("%d ", *p);
    }
    printf("\n");
    return 0;
}