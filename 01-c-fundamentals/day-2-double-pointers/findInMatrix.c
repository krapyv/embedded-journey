#include <stdio.h>
#include <stdint.h>

// Find a value in a 2D array and return a pointer to it via double pointer

// first version: it was close :)
// // i tried to have in the parameters "int matrix[][]" but it caused
// // if (matrix[i][j] == target) to have "expression must be a pointer to a complete object typeC/C++(852)" error
// void find_in_matrix(int matrix[][3], int rows, int cols, int target, int **result_ptr)
// {
//     for (int i = 0; i < cols; i++)
//     {
//         for (int j = 0; j < rows; j++)
//         {
//             if (matrix[i][j] == target)
//             {
//                 *result_ptr = &matrix[i][j];
//                 return; // i missed it for the first time, but we do not need to continue to execute the loop after we fould the target
//             }
//         }
//     }
// }

// i tried to have in the parameters "int matrix[][]" but it caused
// if (matrix[i][j] == target) to have "expression must be a pointer to a complete object typeC/C++(852)" error
void find_in_matrix(int matrix[][3], int rows, int cols, int target, int **result_ptr)
{
    // i've debugged these two versions of the func and i saw that now this function has three "round" in its inner loop (because there are 3 cols), not 2 as in the first version
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (matrix[i][j] == target)
            {
                *result_ptr = &matrix[i][j];
                return; // i missed it for the first time, but we do not need to continue to execute the loop after we fould the target
            }
        }
    }

    // addition to the func
    // if there is no such element
    *result_ptr = NULL;
}
int main(void)
{
    const int rows = 2; // made the vars rows and cols as const because the matrix declaration and initialization complained
    const int cols = 3;

    // findInMatrix.c:26:30: error: variable-sized object may not be initialized except with an empty initializer
    // int matrix[rows][cols] = {{1, 2, 3}, {6, 5, 4}};
    // address = base_address + (i * columns + j) * sizeof(int)
    int matrix[2][3] = {{1, 2, 3}, {6, 5, 4}};

    int *result_ptr = NULL;

    find_in_matrix(matrix, rows, cols, 5, &result_ptr);

    if (result_ptr == NULL)
    {
        printf("There is no such target in the matrix\n");
    }
    else
    {
        printf("after the finding, result_ptr = %p, *result_ptr = %d\n", result_ptr, *result_ptr);
    }

    return 0;
}