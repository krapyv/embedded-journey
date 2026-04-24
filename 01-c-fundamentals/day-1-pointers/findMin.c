#include <stdio.h>
#include <stdint.h>

// Write a function that finds the minimum value AND its index.
// Return the index via a pointer parameter.

// it works!!!
// int find_min(const int *arr, int length, int *min_index)
// {
//     int min = *arr;
//     int index = 0;
//     *min_index = index; // it handles the case when min_index never gets another value in the loop because the element in 0 index is already the min one
//     for (const int *p = arr; p < arr + length; p++, index++)
//     {
//         if (*p < min)
//         {
//             min = *p;
//             *min_index = index; // it works but i do not handle the case if the first element is already the mix
//         }
//     }

//     return min;
// }

// without an index var
int find_min(const int *arr, int length, int *min_index)
{
    int min = *arr;
    *min_index = 0;
    for (const int *p = arr + 1; p < arr + length; p++)
    {
        if (*p < min)
        {
            min = *p;
            *min_index = p - arr;
        }
    }

    return min;
}

int main(void)
{
    int arr[10] = {5, 33, 55, 222, 10004, 10, -1, 10, 5440, 0};
    int length = 10;
    int min_index = 0;

    int min = find_min(arr, length, &min_index);

    printf("The min: %d and the min_index: %d\n", min, min_index);
    return 0;
}