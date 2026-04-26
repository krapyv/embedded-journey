#include <stdio.h>
#include <stdint.h>

// Parse a command string into tokens (like strtok with double pointer state)

// first of all, i need to assume that "double pointer state" is a pointer to the current position that shares between function calls
// secondly, i read the basic info about strtok() func to understand what it is (https://www.w3schools.com/c/ref_string_strtok.php)

// Version 1: segmentation fault (Because *p_result += *p_str is crazy)
// char *next_token(char *str, char delimiter, char **state_ptr)
// {
//     char *p_str = str;
//     char *p_result = str;

//     while (*p_str != delimiter)
//     {
//         *p_result += *p_str;
//         p_str++;
//     }
//     *p_result += '\0';
//     *state_ptr = p_str + 1;

//     return p_result;
// }

char *next_token(char *str, char delimiter, char **state_ptr)
{
    if (str != NULL)
    {
        *state_ptr = str;
    }

    // we could also handle cases when str is more "complex"
    // that means has trailing and leading delimiters
    while (**state_ptr == delimiter)
    {
        (*state_ptr)++;
    }

    if (**state_ptr == '\0')
    {
        return NULL;
    }

    if (*state_ptr == NULL)
    {
        return NULL;
    }

    char *p_start = *state_ptr;

    char *p_end = p_start;

    // in the end of the string there were no delimiters,
    // so the function did not stop and took garbage in, until there was an error or a delimiter
    // while (*p_end != delimiter)

    while (*p_end != delimiter && *p_end != '\0')
    {
        p_end++;
    }
    *p_end = '\0';
    *state_ptr = p_end + 1;

    return p_start;
}

int main(void)
{
    // the biggest problem so far - it is a string literal
    // i tried to modify it and ran consistently in segmentation fault
    // char *arr = "My name is Slim Shady";
    char arr[] = "-----My-name-is-Slim-Shady-----";
    char delimiter = '-';
    char *state_ptr = NULL;

    char *my_result_ptr = next_token(arr, delimiter, &state_ptr);
    while (my_result_ptr != NULL)
    {
        printf("%s\n", my_result_ptr);
        my_result_ptr = next_token(NULL, delimiter, &state_ptr);
    }

    return 0;
}