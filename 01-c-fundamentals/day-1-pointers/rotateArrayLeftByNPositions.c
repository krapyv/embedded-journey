#include <stdio.h>
#include <stdint.h>

// Write a function that rotates an array left by N positions.
// Use pointer arithmetic. No extra array allowed.

void swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// during debugging i found out that i messed up xD
// void rotate_left(int *arr, int length, int positions)
// {
//     int *p1 = arr;
//     int *p2 = arr + positions;
//     // int *p3 = arr + length; // it was the problem. i did not recognized that i was assigning the value out of scope

//     int *p3 = arr + length - 1;

//     // first of all, reverse positions elements in the arr
//     // we need to go forward with p1 and go back with p2
//     while (p1 < p2)
//     {
//         swap(p1, p2);
//         p1++;
//         p2--;
//     }

//     // now we have [2, 1, 3, 4, 5]

//     // secondly, we have to reverse all the remaining elements in the arr
//     // now p1 is arr + positions (After the previous block of incrementations)
//     while (p1 < p3)
//     {
//         swap(p1, p3);
//         p1++;
//         p3--;
//     }

//     // now we have [2, 1, 5, 4, 3]

//     // finally we should reverse the entire array
//     // it means that we need to swap elements symmetrically
//     // element at the first position is swapped with the element at the last position, the second with the second-to-last and so on
//     // now p2 is arr and p1 is arr + length
//     while (p2 < p1)
//     {
//         swap(p2, p1);
//         p2++;
//         p1--;
//     }
// }

void rotate_left(int *arr, int length, int positions)
{
    if (length <= 1 || positions == 0)
        return positions = positions % length; // primarily for N > length cases

    int *p1_1 = arr;
    int *p1_2 = arr + positions - 1;
    // int *p3 = arr + length; // it was the problem. i did not recognized that i was assigning the value out of scope

    int *p2_1 = arr + positions;
    int *p2_2 = arr + length - 1;

    // first of all, reverse positions elements in the arr
    // we need to go forward with p1_1 and go back with p1_2
    while (p1_1 < p1_2)
    {
        swap(p1_1, p1_2);
        p1_1++;
        p1_2--;
    }

    // now we have [2, 1, 3, 4, 5]

    // secondly, we have to reverse all the remaining elements in the arr
    // now p1 is arr + positions (After the previous block of incrementations)
    while (p2_1 < p2_2)
    {
        swap(p2_1, p2_2);
        p2_1++;
        p2_2--;
    }

    // now we have [2, 1, 5, 4, 3]

    // finally we should reverse the entire array
    // it means that we need to swap elements symmetrically
    // element at the first position is swapped with the element at the last position, the second with the second-to-last and so on
    p1_1 = arr;
    p1_2 = arr + length - 1;
    while (p1_1 < p1_2)
    {
        swap(p1_1, p1_2);
        p1_1++;
        p1_2--;
    }
}

int main(void)
{
    int arr[] = {1, 2, 3, 4, 5};
    int length = 5;
    int positions = 2;

    rotate_left(arr, length, positions);

    int *p_arr = arr;

    printf("After the rotation: \n");
    while (p_arr < arr + length)
    {
        printf("%d\n", *p_arr);
        p_arr++;
    }

    // first attempt:
    // After the rotation : 3 4 5 32764 1

    // second attempt:
    // After the rotation : 3 4 5 1 2

    return 0;
}