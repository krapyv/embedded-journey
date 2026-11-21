#ifndef OSAL_TIME_H
#define OSAL_TIME_H

#include <stdint.h>

// function headers
uint32_t osal_get_ms(void);
void osal_delay_ms(uint32_t ms);

#endif // OSAL_TIME_H