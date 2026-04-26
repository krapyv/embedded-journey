#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Write a function that finds the first pair of elements that sum to a target value.
// Return pointers to the two elements via double pointers.

int find_pair(int *arr, int length, int target, int **p1, int **p2)
{
    // a minor validation check
    if (arr == NULL || length < 2)
    {
        return 0;
    }

    for (int *p_arr = arr; p_arr < arr + length; p_arr++)
    {
        // we need to check if the sum of each element of the array with the next elements in that array would give us a target value
        // {1, 4, 3, 6, 2, 5} => 1 + 4, 1 + 3, 1 + 6 .... => if there is no pair that gives us the target value => the next element: 4 + 3, 4 + 6 and so on.

        for (int *next = p_arr + 1; next < arr + length; next++)
        {
            if (*p_arr + *next == target)
            {
                *p1 = p_arr;
                *p2 = next;

                return 1;
            }
        }
    }

    return 0;
}

int main(void)
{
    int arr[] = {1, 4, 3, 6, 2, 5};

    int length = 6;
    int target = 11;

    int *a;
    int *b;

    if (find_pair(arr, length, target, &a, &b))
    {
        printf("The pair of elements that sum to a target value is found!\n\n");
        printf("%d + %d = %d\n", *a, *b, target);
    }
    else
    {
        printf("There is no pair of numbers that sum to a target value!\n\n");
    }
}