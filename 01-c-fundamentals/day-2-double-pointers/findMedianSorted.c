#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// Write a function that finds the median of two sorted arrays.
// Use pointers to traverse.

void sortArrayElement(int *arr, int length, int *elem)
{
    // let's assume that the elem is the smallest value of the rest of the array (so we are assuming that it is not going to change places with any other array element)
    int *smaller = elem;
    for (int *p_elem = elem + 1; p_elem < arr + length; p_elem++)
    {
        if (*smaller > *p_elem)
        {
            int tmp = *smaller;
            *smaller = *p_elem; // we need it just not to lose the value of the smaller pointer
            *p_elem = tmp;
        }
    }

    // it is cool that the value of *elem i pass to the function is changes too
    // love pointers
}

double calc_median(const int *array, int length)
{
    // if an array have an odd number of elements, a median is the element that splits the array into two equal-sized arrays
    // if an array have an even number of elements, a median is the sum of the two central elements of the erray divided by 2
    double median = 0;

    // the number of elements is an even number, so we need to find two central elements, to sum them up and to divide by 2
    if (length % 2 == 0)
    {
        // to find two central elements, first of all, we need to find their indexes
        // in simpler words, we need to find how many elements there are before the first of our central elements and subtract 1 because that 1 is our first central element
        int before = length / 2 - 1;

        // int centralPositions[] = {before + 1, before + 2}; // it will be more comfy to operate on indexes, not on real positions and every time to subtract 1
        int centralIndexes[] = {before, before + 1};

        for (int *pos = centralIndexes; pos < centralIndexes + 2; pos++)
        {
            median += *(array + *pos);
        }

        median /= 2;
    }
    // otherwise we need to find the central element of the array that splits the array in half
    else
    {
        // to find the position of that central element, we need to find how many elements there are before the central one
        // we just can to divide the number of the elements by 2 and by using Math.floor() to round a number down to the nearest interger
        // the central element will be the next one

        int before = floor(length / 2); // it will be our central element index (for example, 13 elements => 6.5 => 6 elements before the central one => 6 index of the element but the 7th in the order)

        median = (double)*(array + before);
    }

    return median;
}

double find_median_sorted(const int *arr1, int len1, const int *arr2, int len2)
{
    if (len1 == 0 && len2 == 0)
        return -1;
    else if (len1 == 0)
    {
        // we could calculate the median of the second array
        return calc_median(arr2, len2);
    }
    else if (len2 == 0)
    {
        // we could calculate the median of the first array
        return calc_median(arr1, len1);
    }

    int new_length = len1 + len2;

    // we need to merge these two arrays to be able to find its median
    int *merged_array = (int *)malloc(new_length * sizeof(int));

    int *p_merged_array = merged_array; // just not to lose the pointer to the beginning of the array

    // add the arr1 elements to the merged_array
    for (const int *p1 = arr1; p1 < arr1 + len1; p1++)
    {
        *p_merged_array = *p1;
        p_merged_array++;
    }

    // add the arr2 elements to the merged_array
    for (const int *p2 = arr2; p2 < arr2 + len2; p2++)
    {
        *p_merged_array = *p2;
        p_merged_array++;
    }

    // i introduced p_merged_array to avoid the rist with that
    // // we need to return to the first element of the arr
    // // because otherwise we are "losing" the hold on the first position of the arr
    // // and the futher code would operate not on our allocated array, but on some other memory
    // // and we will get malloc(): corrupted top size
    // merged_array -= new_length;

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

    for (int *p = merged_array; p < merged_array + new_length; p++)
    {
        sortArrayElement(merged_array, new_length, p);
    }

    // just to confirm that my array is sorted
    printf("The sorted merged array:\n");
    for (int *p = merged_array; p < merged_array + new_length; p++)
    {
        printf("%d ", *p);
    }

    double median = calc_median(merged_array, new_length);

    free(merged_array);

    return median;
}

int main(void)
{
    int arr1[] = {1, 3, 5, 8, 12};
    int arr2[] = {2, 4, 6, 9};

    int len1 = 0;
    int len2 = 1;

    double med = find_median_sorted(arr1, len1, arr2, len2);

    printf("The median of the array: %.1f\n", med);

    return 0;
}