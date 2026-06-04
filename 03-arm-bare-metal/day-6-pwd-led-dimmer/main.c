#include "pwm_driver.h"
#include "stm32f411.h"

int main(void)
{
    PWM_HandleTypeDef pwm;
    pwm.Instance = TIM2;
    pwm.Port = GPIOA;
    pwm.pin = 5;
    pwm.af = GPIO_AF1;
    pwm.Period = 15999;
    pwm.Pulse = 8000;
    pwm.channel = 1;

    led_init(&pwm);
    pwm_init(&pwm);

    uint32_t brightness = 1000;
    int32_t step = 50;

    while (1)
    {
        brightness += step;

        if (brightness >= 15000 || brightness <= 500)
        {
            step = -step;
        }

        pwm.Instance->CCR[pwm.channel - 1] = brightness;

        for (volatile int i = 0; i < 100000; i++)
            ;
    }

    return 0;
}