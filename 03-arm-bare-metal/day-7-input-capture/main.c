#include <stdio.h>
#include "uart.h"
#include "core_cm4.h"
#include "stm32f411.h"
#include "input_capture_driver.h"

extern IC_HandleTypeDef input_capture; // borrowing the variable

int main(void)
{
    // initializing the system clock and console debugging
    timer_init();
    usart2_init();

    printf("--- System Boot: Drivers Initialized ---\n");

    // configuring the Input Capture structure
    input_capture.Instance = TIM3;
    input_capture.Port = GPIOB;
    input_capture.pin = 0;
    input_capture.af = GPIO_AF2;
    input_capture.channel = 3;
    input_capture.polarity = IC_FE;

    input_capture.data.overflow_count = 0;
    input_capture.data.first_timestamp = 0;
    input_capture.data.second_timestamp = 0;
    input_capture.data.delta = 0;

    // initializing the hardware channels and enabling interrupts
    nvic_interrupts_init(&input_capture);
    input_capture_init(&input_capture);

    while (1)
    {
        // if the first_timestamp has reset to 0, but the delta contains values, a full timing cycle finished and had been captured by the ISR
        if (input_capture.data.first_timestamp == 0 && input_capture.data.delta != 0)
        {
            uint32_t raw_ticks = input_capture.data.delta;

            input_capture.data.delta = 0;

            // we are using the Processor Clock with 16 MHz with prescaler = 0
            uint32_t total_ms = raw_ticks / 16000;
            uint32_t passed_seconds = total_ms / 1000;
            uint32_t remainder_ms = total_ms % 1000;

            printf("Button Hold Duration: %lu s %lu ms (Raw Ticks: %lu)\n\r", passed_seconds, remainder_ms, raw_ticks);
            fflush(stdout);
        }
    }

    return 0;
}