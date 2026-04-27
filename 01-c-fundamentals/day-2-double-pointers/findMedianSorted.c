#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Write a function that finds the median of two sorted arrays.
// Use pointers to traverse.

double find_median_sorted(const int *arr1, int len1, const int *arr2, int len2)
{
    int new_length = len1 + len2;

    // we need to merge these two arrays to be able to find its median
    int *merged_array = (int *)malloc(new_length * sizeof(int));

    int *p_merged_arr1 = NULL;
    int *p_merged_arr2 = NULL;

    // add the arr1 elements to the merged_array
    for (const int *p1 = arr1; p1 < arr1 + len1; p1++)
    {
        *merged_array = *p1;
        if (p1 == arr1)
        {
            p_merged_arr1 = merged_array;
        }
        merged_array++;
    }

    // add the arr2 elements to the merged_array
    for (const int *p2 = arr2; p2 < arr2 + len2; p2++)
    {
        *merged_array = *p2;
        if (p2 == arr2)
        {
            p_merged_arr2 = merged_array;
        }
        merged_array++;
    }

    // now we have merged unsorted array
    // we need to sort it

    // it does not work. in my head it made sense, but in reality it is nonsense.
    // for example, {4, 6, 2, 5, 9};
    // assume that we have pointers to 4 (p1) (the first elem of the old arr1) and to 2 (p2) (the first elem of the old arr2)
    // then if we are going to compare *p1 = 4 and *p2 = 2, we should interchange their positions
    // {2, 6, 4, 5, 9}, p1++ => *p1 = 6, *p2 = 4;
    // the next iteration: *p1 (6) > *p2 (4) => {2, 4, 6, 5, 9}, p1++ => *p1 = 5, *p2 = 5, p1 == p2 => p2++;
    // *p1 = 6, *p2 = 5
    // *p1 (6) > *p2 (5) => {2, 4, 5, 6, 9} => p1++ => p1 == p2 (*p1 == *p2) => p2++;
    // *p1 = 6 > *p2 = 9 => nothing changes. if there are another elements after p2, we should stop, because they are already sorted

    // while ((p_merged_arr1 < merged_array + new_length) && (p_merged_arr2 < merged_array + new_length)) {
    //     if (*p_merged_arr1 > *p_merged_arr2) {
    //         int tmp = *p_merged_arr1;
    //         *p_merged_arr1 = *p_merged_arr2;
    //         *p_merged_arr2 = tmp;

    //         p_merged_arr1++;
    //     }
    //     else if (*p_merged_arr1 < *p_merged_arr2) {
    //         p_merged_arr1++;
    //     }

    // }

    // it seems to work!!!

    // we need to create a function that would compare the element with all elements in the array (that have bigger index)
    // if there is an element that is smaller that our elem, we need to write its address and continue to compare the values
    // if there is another, even smaller element, we are interchanging the positions with it

    // {2, 4, 7}  {1, 3, 5, 8} => {2, 4, 7, 1, 3, 5, 8}, int *smaller;
    // 2: 2 < 4 => 2 < 7 => 2 > 1  (smaller = 0x(1)) => 2 < 3 ...
    // interchange the values => {1, 4, 7, 2, 3, 5, 8}
    // 4: 4 < 7 => 4 > 2 (smaller = 0x(2)) => 4 > 3 (but *smaller = 2 < 3) => 4 < 5 ...
    // interchange the values => {1, 2, 7, 4, 3, 5, 8}
    // 7: 7 > 4 (smaller = 0x(4)) => 7 > 3 (*smaller = 4 > 3) => (now *smaller = 3) => 7 > 5 (but *smaller = 3 < 5, so no change) => ...
    // interchange the values => {1, 2, 3, 4, 7, 5, 8}
    // 4: 4 < 7 => 4 < 5 => 4 < 8
    // 7: 7 > 5 => (*smaller = 5) => 7 < 8
    // interchange the values: {1, 2, 3, 4, 5, 7, 8}
    // 7: 7 < 8
    // 8: no values to compare with
    // now the array is sorted
}

int main(void)
{
    int arr1[] = {1, 2};
    int arr2[] = {3, 4};

    int len1 = 2;
    int len2 = 2;

    double med = find_median_sorted(arr1, len1, arr2, len2);
}