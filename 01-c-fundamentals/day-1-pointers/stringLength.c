#include <stdio.h>
#include <stdint.h>

// Task 1.1: Write a function that returns the
// length of a string using only pointers (no strlen, no index variables).

// we could go with size_t instead of int
int string_length(const char *str)
{
    // we need to count all the characters before the termination symbold (\0)
    int length = 0;

    while (*str != '\0')
    {
        length++;
        str++;
    }

    return length;
}

// alternative

size_t string_length1(const char *str)
{
    size_t length = 0;
    while (*str++)
    {
        length++;
    }

    return length;
}

int main(void)
{
    // well, i wanted to initialize a char var with string (basically an array of chars). not my best idea ever
    // char string = "How are you?";
    char string[] = "How are you?";                                      // 12 characters
    printf("The length of the passed is %lu\n", string_length1(string)); // it gives me 12 characters

    return 0;
}