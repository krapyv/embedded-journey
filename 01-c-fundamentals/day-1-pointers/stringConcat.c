#include <stdio.h>
#include <stdint.h>

// Task 1.3: Write a function that concatenates two strings
// using only pointers (no strcat).

// dest ends with null terminator
// src ends with null terminator
// so we should remove dest terminator and only use it at the end of the new string
void string_concat(char *dest, const char *src)
{
    // we should simply get the pointer on the last symbol in dest
    // then we should in the loop (until the src string ends up with '\0')
    // add all the chars from src to the end of the dest (dest++)

    if (*src == '\0')
        return;

    while (*dest != '\0')
    {
        dest++;
    }

    while (*src != '\0')
    {
        *dest++ = *src++;
    }

    *dest = '\0';
}

int main(void)
{
    // again tried to assign a string to a char xD
    // char dest = "My string";
    char dest[50] = "My string";

    const char *src = "New string";

    string_concat(dest, src);

    printf("after concatination: %s\n", dest);

    return 0;
}