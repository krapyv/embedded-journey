#include <stdio.h>
#include <stdint.h>

/*

Write a function that takes a string (null-terminated) and removes consecutive duplicate characters in-place using only pointers (no index variables).

Example:

Input: "aabbccddeeff" → Output: "abcdef"

Input: "hello" → Output: "helo" (double 'l' removed)

Use pointer arithmetic. No strlen. Stop at \0.

*/

// first of all, strings in C are just char arrays terminated by a null character.

// void removeDuplicates(char str[])
// {
//     char *p_str = str;
// printf("*p_str: %c\n", *p_str); // points to the first char

// we could traverse the string
// while (*p_str != '\0')
// {
//     printf("inside while: %c\n", *p_str);
//     p_str++;
// }

// char tmp = *p_str;
// while (*p_str != '\0')
// {
//     if (tmp == *(p_str++))
//     printf("inside while: %c\n", *p_str);
//     p_str++;
// }

// wanted to find how many chars in the array
// unfortunately it is not possible
// printf("sizeof(str): %lu\n", sizeof(str) / sizeof(str[0]));

// we track every symbol and see where the duplicates are

// char tmp = *p_str;
// while (*p_str != '\0')
// {
//     char *p_tmp = p_str + 1;
//     printf("p_str and p_tmp and tmp: %c %c %c\n", *p_str, *p_tmp, tmp);

//     if (tmp == *p_tmp)
//     {
//         printf("duplicate: %c\n", *p_tmp);
//     }
//     else
//     {
//         printf("not a duplicate: %c\n", *p_tmp);
//     }
//     tmp = *(++p_str);
// }

// i tried to make it work as needed, but it is failed
// char tmp = *p_str;
// while (*p_str != '\0')
// {
//     char *p_tmp = p_str + 1;
//     // printf("p_str and p_tmp and tmp: %c %c %c\n", *p_str, *p_tmp, tmp);

//     if (tmp == *p_tmp)
//     {
//         printf("duplicate: %c\n", *p_tmp);
//         char *p = p_tmp;
//         while (*p != '\0')
//         {
//             p = (p + 1);
//             printf("*p: %c\n", *p);
//                     }
//     }
//     else
//     {
//         printf("not a duplicate: %c\n", *p_tmp);
//     }
//     tmp = *(++p_str);
// }

// i tried to make it work as needed, but it is failed
// char *p_tmp = p_str + 1;
// char tmp = *p_str;
// while (*p_str != '\0')
// {
//     // printf("p_str and p_tmp and tmp: %c %c %c\n", *p_str, *p_tmp, tmp);
//     p_str++;
//     p_tmp++;
//     if (*p_str == tmp)
//     {
//         char *p = p_str;
//         while (*p_tmp != '\0')
//         {
//             *p_str = *p_tmp;
//             p_str++;
//             p_tmp++;
//         }

//         *p_str = *p;
//         *p_tmp = *p_str + 1;
//     }
//     else
//     {
//         printf("not a duplicate: %c\n", *p_tmp);
//     }
//     tmp = *p_str;
// }
// }

void removeDuplicates(char str[])
{
    char *p_str = str;

    if (*p_str == '\0')
        return;

    char *write = p_str;
    char *read = p_str;

    while (*read != '\0')
    {
        if (*write != *read)
        {
            write++;
            *write = *read;
        }
        read++;
    }

    *(write + 1) = '\0';
}

int main(void)
{
    char str[] = "Hellloo";
    removeDuplicates(str);

        return 0;
}