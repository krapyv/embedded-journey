#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Write a function that merges two sorted arrays into a new sorted array.
// Use pointer arithmetic to traverse both arrays simultaneously.

int *merge_sorted(const int *arr1, int len1, const int *arr2, int len2, int *new_length)
{
    // firstly, we must assume that all the arrays we have do not have identical elements
    int merged_size = len1 + len2;

    // secondly, let's allocate our new merged array
    int *merged_arr = (int *)malloc(merged_size * sizeof(int));

    // i guess we are going to traverse both arrays, so we might need to have two pointers (because if an element from the e.g. second array would be smaller that an element at the same index from the first array, than we are going to push that smaller element to the merged array, move the pointer p_arr2 to the next further position and compare the value in that position with the value from the first array (the pointer p_arr1 has not changed))
    const int *p_arr1 = arr1; // add the const keyword because of the warning warning: initialization discards ‘const’ qualifier from pointer target type [-Wdiscarded-qualifiers]
    const int *p_arr2 = arr2;

    // we also need to determine which array has the first element (the smallest number) of the new merged array

    // well, it was a fun idea, but i suddenly realized that we have pointers to the first elements of the arrays and the lengths of these arrays, so we do not need to calculate their shared length and other things xD
    // // the next problem: the arrays could have different lengths
    // // so let's take the smaller length
    // int sharedLength;
    // if (len1 >= len2) {
    //     sharedLength = len1;
    // } else {
    //     sharedLength = len2;
    // }

    const int *end1 = arr1 + len1;
    const int *end2 = arr2 + len2;
    int *p_merged = merged_arr;

    while ((p_arr1 < end1) && (p_arr2 < end2))
    {
        // we are dereferencing these pointers because we do not want to compare their addresses, we want to compare their values
        if (*p_arr1 < *p_arr2)
        {
            // we dereferencing *merged_arr because we do not want to assign an address of the other array's element as an element of this already allocated array
            *p_merged = *p_arr1;
            p_merged++;
            p_arr1++;
        }
        else if (*p_arr1 > *p_arr2)
        {
            *p_merged = *p_arr2;
            p_merged++;
            p_arr2++;
        }
        else
        {
            // equals
            *p_merged = *p_arr1;
            p_merged++;
            p_arr1++;
            p_arr2++;
        }
    }

    // in case there are some elements in the arrays when the while loop ended (it means that these elements are bigger, so should be the last one in the merged array)

    while (p_arr1 < end1)
    {
        *p_merged = *p_arr1;
        p_merged++;
        p_arr1++;
    }

    while (p_arr2 < end2)
    {
        *p_merged = *p_arr2;
        p_merged++;
        p_arr2++;
    }

    // example: a = {1, 3, 6}  b= {2, 4, 6};
    // 1 < 2 y => merged_arr = {1}, a++; *a = 3; *b = 2;
    // 3 < 2 n => merged_arr = {1, 2}, b++; *a = 3; *b = 4;
    // 3 < 4 y => merged_arr = {1, 2, 3}, a++; *a = 6; *b = 4;
    // 6 < 4 n => merged_arr = {1, 2, 3, 4}, b++; *a = 6; *b = 6;
    // 6 < 6 n => 6 > 6 n => 6 == 6 => merged_arr = {1, 2, 3, 4, 6}, a++, b++; return;

    // example: a = {1, 3, 5, 6}  b= {2, 4, 6};
    // 1 < 2 y => merged_arr = {1}, a++; *a = 3; *b = 2;
    // 3 < 2 n => merged_arr = {1, 2}, b++; *a = 3; *b = 4;
    // 3 < 4 y => merged_arr = {1, 2, 3}, a++; *a = 5; *b = 4;
    // 5 < 4 n => merged_arr = {1, 2, 3, 4}, b++; *a = 5; *b = 6;
    // 5 < 6 y => merged_arr = {1, 2, 3, 4, 5}, a++; *a = 6; *b = 6;
    // 6 < 6 n => 6 > 6 n => 6 == 6 => merged_arr = {1, 2, 3, 4, 5, 6}, a++, b++; return;

    // then i want to check whether there are free "element spaces" in the array
    int diff = (merged_arr + merged_size) - p_merged;

    // if there are trailing elements, resize the array;
    if (diff != 0)
    {
        merged_size -= diff;
        // On success, returns a pointer to the beginning of newly allocated memory.
        // To avoid a memory leak, the returned pointer must be deallocated with std::free or std::realloc.
        // The original pointer ptr is invalidated and any access to it is undefined behavior (even if reallocation was in-place).
        merged_arr = realloc(merged_arr, (merged_size * sizeof(int)));
    }

    // // pass back to the main new size of the array
    *new_length = merged_size;

    return merged_arr;
}

int main(void)
{
    int arr1[] = {1, 5, 6, 9, 22};
    int len1 = 5;

    int arr2[] = {1, 2, 4, 5, 7, 9, 10};
    int len2 = 7;

    // a double pointer was not a great idea
    // int *merged_size = NULL;

    int merged_size = 0;

    int *merged_arr = merge_sorted(arr1, len1, arr2, len2, &merged_size);

    printf("The merged size: %d\n\n", merged_size);

    for (int *p = merged_arr; p < merged_arr + merged_size; p++)
    {
        printf("%d ", *p);
    }

    free(merged_arr);
}