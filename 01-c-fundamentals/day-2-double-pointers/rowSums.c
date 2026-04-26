#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Write a function that takes a 2D array (pointer to pointers) and returns an array of row sums.

int *row_sums(int **matrix, int rows, int cols)
{
    if (rows == 0 || cols == 0)
    {
        return NULL;
    }

    if (matrix == NULL || *matrix == NULL)
    {
        return NULL;
    }

    // we could also add
    for (int i = 0; i < rows; i++)
    {
        if (matrix[i] == NULL)
            return NULL;
    }

    // we need to allocate the dynamic array
    int *sums = (int *)malloc(rows * sizeof(int));
    for (int **p1 = matrix; p1 < matrix + rows; p1++)
    {
        int row_sum = 0;
        for (int *p2 = *p1; p2 < *p1 + cols; p2++)
        {
            row_sum += *p2;
        }
        // p1 - matrix in my mind will give us the index where we could store our row_sum
        *(sums + (p1 - matrix)) = row_sum; // or sums[p1-matrix] = row_sum;
    }

    return sums;
}

int main(void)
{
    int rows = 3, cols = 3;

    // i guess i could not initialize the matrix like a scalar array
    // int *matrix[] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    int row0[] = {1, 2, 3};
    int row1[] = {4, 5, 6};
    int row2[] = {7, 8, 9};

    int *matrix[] = {row0, row1, row2};

    int *sums = row_sums(matrix, rows, cols);

    if (sums == NULL)
    {
        printf("I could not sum the row values of this array\n\n");
    }
    else
    {
        for (int *p_sums = sums; p_sums < sums + rows; p_sums++)
        {
            printf("%d\n", *p_sums);
        }
        free(sums);
    }

    return 0;
}