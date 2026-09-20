#include "stack.h"

bool Stack_init(Stack_t *stack, uint32_t *buf, uint32_t size)
{
    if (size == 0)
    {
        return false;
    }

    stack->data = buf;
    stack->size = size;
    stack->top = -1;

    return true;
}

bool Stack_push(Stack_t *stack, uint32_t element)
{
    if (stack->top >= (int)stack->size - 1)
    {
        return false; // overflow
    }

    stack->data[++stack->top] = element;

    return true;
}

bool Stack_pop(Stack_t *stack, uint32_t *element)
{
    if (stack->top < 0)
    {
        return false; // underflow, no elements
    }

    *element = stack->data[stack->top--];

    return true;
}

void Stack_clear(Stack_t *stack)
{
    stack->top = -1;
}