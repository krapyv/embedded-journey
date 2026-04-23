#include <stdio.h>
#include <stdint.h>

// Task 1.2: Write a function that
// copies one string to another using only pointers (no strcpy).

void string_copy(char *dest, const char *src)
{
    while (*src != '\0')
    {
        *dest++ = *src++;
    }

    *dest = '\0';
}

int main(void)
{
    // BUG: I did not initialize the pointer, so it has just garbage.
    // I need to use array or dynamically allocate the memory
    // char *dest;

    char dest[60];
    char *p_dest = dest;
    const char *src = "Hello World!";

    string_copy(dest, src);

    printf("The dest:\n");
    while (*p_dest != '\0')
    {
        printf("%c", *p_dest);
        p_dest++;
    }

    // or

    printf("The dest:\n%s\n", dest);
    return 0;
}