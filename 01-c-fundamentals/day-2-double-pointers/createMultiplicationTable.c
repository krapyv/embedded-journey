#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Write a function that dynamically allocates a 2D array (multiplication table) and returns it via a double pointer.

int **create_multiplication_table(int n)
{
    // i forgot to add "malloc" ;) this caused the line "matrix[i][j] = (i + 1) * (j + 1);" to have segmentation fault xD
    // int **matrix = (int **)(n * n * sizeof(int));

    // well, it is a very incorrect 2D array initialization. i do not know why i went with this way in the first place
    // int **matrix = (int **)malloc(n * n * sizeof(int));

    // the correct way
    // first of all, we need to allocate the memory for rows
    // int **matrix = (int **)malloc(n * sizeof(int));
    if (n <= 0)
    {
        return NULL;
    }

    int **matrix = (int **)malloc(n * sizeof(int *));

    // additional checking
    // if allocation went wrong
    if (matrix == NULL)
        return NULL;

    for (int i = 0; i < n; i++)
    {
        // secondly we are initializing the memory for cols
        matrix[i] = (int *)malloc(n * sizeof(int));

        // if allocation of memory for a row went wrong
        if (matrix[i] == NULL)
        {
            // we should clean up already allocated rows
            for (int k = 0; k < i; k++)
                free(matrix[k]);
            free(matrix);
            return NULL;
        }

        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = (i + 1) * (j + 1);
        }
    }

    return matrix;
}

int main(void)
{
    int n = 5;
    int **table = create_multiplication_table(n);

    // it is wrong, because i need an adress of table, not the value behind it xD
    // for (int **p1 = **table; p1 < table + n; p1++)

    if (table == NULL)
    {
        printf("Something went wrong\n\n");
        return -1;
    }

    for (int **p1 = table; p1 < table + n; p1++)
    {
        for (int *p2 = *p1; p2 < *p1 + n; p2++)
        {
            printf("%d\t", *p2);
        }
        printf("\n");
    }

    // free(table); // well, it is too soon to be happy

    for (int i = 0; i < n; i++)
        free(table[i]);
    free(table);

    return 0;
}