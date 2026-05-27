#include "uart.h"
#include "ring_buffer.h"
#include "led.h"
#include "iwdg.h"

extern RingBuffer_t rx_buffer; // access the global storage from uart.c
extern volatile uint32_t ms_passed;

#define RCC_CSR_REG *(volatile uint32_t *)(0x40023800 + 0x74)

int main(void)
{
    usart2_init();
    timer_init();

    LED_HandleTypeDef led_pa5;
    led_pa5.moder_reg = (volatile uint32_t *)(0x40020000 + 0x00);
    led_pa5.odr_reg = (volatile uint32_t *)(0x40020000 + 0x14);
    led_pa5.rcc_clk_reg = (volatile uint32_t *)(0x40023800 + 0x30);
    led_pa5.pin = 5;
    led_pa5.rcc_bit = 0;

    led_init(&led_pa5);

    usart2_write_string("\r\n--- MCU Boot Sequence ---\r\n");

    // check IWDG_RSTF (bit 29)
    if (RCC_CSR_REG & (1 << 29))
    {
        usart2_write_string("[ALERT] System recovered from a WATCHDOG RESET!\r\n");

        // rapidly flash the led 5 timers (via toggling its ODR bit) to show a recovery occurred
        for (int i = 0; i < 10; i++)
        {
            *led_pa5.odr_reg ^= (1 << led_pa5.pin);

            for (volatile int d = 0; d < 100000; d++)
                ;
        }

        // clear the reset flag by writing to the RMVF bit 24 (according to the manual)
        RCC_CSR_REG |= (1 << 24);
    }
    else
    {
        usart2_write_string("[INFO] System booted normally (Clear Power-On).\r\n");
    }

    IWDG_HandleTypeDef watchdog;
    watchdog.IWDG_BASE_REG = (volatile uint32_t *)(0x40003000);

    // using Prescaler 64 with LSI (32 kHz) = watchdog clock runs at 500 Hz (2 ms per tick)
    watchdog.prescaler_val = IWDG_PRESCALER_64;

    // reload value 1000 = 1000 ticks * 2 ms = 2000 ms (2 seconds timeout window)
    watchdog.reload_val = 1000;

    usart2_write_string("[INFO] Arming the Independent Watchdog...\r\n");
    IWDG_init(&watchdog);
    usart2_write_string("[INFO] Watchdog Active. Running normal loop for 4 seconds.\r\n");

    uint32_t last_transmission = 0;
    led_on(&led_pa5);

    while (1)
    {
        // asynchronous non-blocking TX timing
        if (ms_passed - last_transmission >= 500)
        {
            usart2_write_string("System Status: OK (Kicking Dog)\r\n");
            last_transmission = ms_passed;
        }

        // the fault simulation
        // after 4000ms, simulate a software lockup
        if (ms_passed >= 4000)
        {
            usart2_write_string("\r\n!!! CRITICAL FAILURE: Software entry into infinite loop !!!\r\n");
            usart2_write_string("Stopping all watchdog kicks. Reset expected in 2.0s... \r\n\r\n");

            led_off(&led_pa5);

            while (1)
            {
                // permanent dead loop trapping the CPU core.
                // since we are stuck here, IWDG_kick() is never reached.
            }
        }

        // under normal healthy conditions, we are kicking the watchdog
        IWDG_kick(&watchdog);
    }
}