// 2

#include <stdio.h>
#include <stdint.h>

// Version 1:
// i should not have dereference arr + i, because it does not make sense to assign *(arr+length-i) to other
// it is bad xD
// void reverseArray(int *arr, int length) {
//     // [1,2,3,4,5] -> [5, 4, 3, 2, 1]
//     // printf("*arr: %d\n", *arr); // it gives me 1

//     for (int i = 0; i < length; ++i) {
//     int temp = *(arr+i);

//     // *(arr+i) = *(arr+length-i);
//     *(arr+length-i) = temp;

//     printf("*(arr+i): %d\n", *(arr+i));
//     }

// }

// Version 2: Works !!!
// void reverseArray(int *arr, int length)
// {
//     for (int i = 0; i < length / 2; ++i)
//     {

//         int tmp = *(arr + length - i - 1);
//         *(arr + length - i - 1) = *(arr + i);
//         *(arr + i) = tmp;

//         printf("*arr: %d\n", *(arr + i));
//         // the core
//         // printf("*(arr+i): %d\n", *(arr+i - 1));
//         // printf("*(arr+length - i): %d\n", *(arr+length - i));
//     }
// }

void reverseArray(int *arr, int length)
{
    int *left = arr;
    int *right = arr + length - 1;

    while (left < right)
    {
        int tmp = *left;
        *left = *right;
        *right = tmp;

        left++;
        right--;
    }
}

int main(void)
{
    // well, i tried to initialize the pointer with array
    // it did not work
    // int *arr = {1,2,3,4,5};
    int number = 10;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    // again i tried to initialize the pointer with an int
    // int *p_arr = arr[0];

    // now i initialized the pointer with an address of the first elem of the array
    int *p_arr = &arr[0];
    reverseArray(p_arr, number);
    int *p;
    for (p = arr; p < arr + number; p++)
    {
        printf("%d\n", *p);
    }
    return 0;
}