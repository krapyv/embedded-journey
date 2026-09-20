#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t *data;
    uint32_t size;
    int top; // index of the last populated element (-1 if empty)
} Stack_t;

bool Stack_init(Stack_t *stack, uint32_t *buf, uint32_t size);
bool Stack_push(Stack_t *stack, uint32_t element);
bool Stack_pop(Stack_t *stack, uint32_t *element);
void Stack_clear(Stack_t *stack);

#endif // STACK_H