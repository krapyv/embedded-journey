#include "stm32f411.h"
#include "input_capture_driver.h"

void input_capture_init(IC_HandleTypeDef *ic)
{
    if (ic->pin > 15 || ic->channel > 4)
        return;

    // enabling the clock for the Port
    if (ic->Port == GPIOA)
    {
        RCC->AHB1ENR |= (1 << 0);
    }
    else if (ic->Port == GPIOB)
    {
        RCC->AHB1ENR |= (1 << 1);
    }

    // setting the MODER to AF (10)

    // every pin takes two bits in the MODER
    // e.g. pin 0 takes bits 0 and 1
    // so the least significant bit has a position: ic->pin * 2
    // e.g. pin 0 -> lsb: 0, pin 9 -> lsb: 9 * 2 = 18;

    // clearing the bits
    // 11 = 2^1 + 2^0 = 3 = 0x3
    ic->Port->MODER &= ~(0x3 << (ic->pin * 2));

    // setting the bits to 10
    // 10 = 2^1 = 0x2
    ic->Port->MODER |= (0x2 << (ic->pin * 2));

    // setting the PB0 to the passed AF value
    // each pin in both AFRL and AFRH takes 4 bits
    // e.g., pin 1 takes bits 7:4
    if (ic->pin < 8)
    {
        // clearing the bits
        // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
        ic->Port->AFRL &= ~(0xF << (ic->pin * 4));

        // setting the bits to the passed AF value
        ic->Port->AFRL |= (ic->af << (ic->pin * 4));
    }
    else
    {
        // we are subtracking 8 from the pin number
        // to prevent overfloating of uint32_t (because e.g. pin number 8 * 4 = 32 => 0xF << 32 <----- does not make much sense)

        // clearing the bits
        // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
        ic->Port->AFRH &= ~(0xF << ((ic->pin - 8) * 4));

        // setting the bits to the passed AF value
        ic->Port->AFRH |= (ic->af << ((ic->pin - 8) * 4));
    }

    // enabling the clock for passed Instance
    if (ic->Instance == TIM2)
    {
        RCC->APB1ENR |= (1 << 0);
    }
    else if (ic->Instance == TIM3)
    {
        RCC->APB1ENR |= (1 << 1);
    }
    else if (ic->Instance == TIM4)
    {
        RCC->APB1ENR |= (1 << 2);
    }
    else if (ic->Instance == TIM5)
    {
        RCC->APB1ENR |= (1 << 3);
    }

    // setting bits in CCMR
    if (ic->channel == 1 || ic->channel == 2)
    {
        // setting CCxS bits to 01 (input, mapped on TIx)

        // if it is a channel 1 -> (1 - 1) * 8 = 0
        // if it is a channel 2 -> (2 - 1) * 8 = 8
        uint32_t ccs_pos = ((ic->channel - 1) * 8);

        // clearing the bits
        // 0x3 = 0011

        ic->Instance->CCMR1 &= ~(0x3 << ccs_pos);

        // setting the bits
        // 01 = 0x1
        ic->Instance->CCMR1 |= (0x1 << ccs_pos);

        // setting ICxF (filtering) to 1111 (fDTS / 32, N=8)
        // channel 1 - bits 7:4
        // channel 2 - bits 15:12

        // if channel 1 -> ((1 - 1) * 8)+ 4 = 4
        // if channel 2 -> ((2 - 1) * 8)+ 4 = 12
        uint32_t icf_pos = ((ic->channel - 1) * 8) + 4;

        // clearing the bits
        // 1111 = 15 = 0xF
        ic->Instance->CCMR1 &= ~(0xF << icf_pos);

        // setting the bits
        // 1111 = 0xF
        ic->Instance->CCMR1 |= (0xF << icf_pos);
    }
    else
    {
        // setting CCxS bits to 01 (input, mapped on TIx)

        // if it is a channel 3 -> (3 - 3) * 8 = 0
        // if it is a channel 4 -> (4 - 3) * 8 = 8
        uint32_t ccs_pos = ((ic->channel - 3) * 8);

        // clearing the bits
        // 0x3 = 0011
        ic->Instance->CCMR2 &= ~(0x3 << ccs_pos);

        // setting the bits
        // 01 = 0x1
        ic->Instance->CCMR2 |= (0x1 << ccs_pos);

        // setting ICxF (filtering) to 1111 (fDTS / 32, N=8)
        // channel 3 - bits 7:4
        // channel 4 - bits 15:12

        // if channel 3 -> ((3 - 3) * 8) + 4 = 4
        // if channel 4 -> ((4 - 3) * 8) + 4 = 8 + 4 = 12
        uint32_t icf_pos = ((ic->channel - 3) * 8) + 4;

        // clearing the bits
        // 1111 = 15 = 0xF
        ic->Instance->CCMR2 &= ~(0xF << icf_pos);

        // setting the bits
        // 1111 = 0xF
        ic->Instance->CCMR2 |= (0xF << icf_pos);
    }

    // set bits in CCER

    // enable capture/compare output for corresponding channel (set to 1)
    // channel 1 - bit 0; channel 2 - bit 4; channel 3 - bit 8; channel 4 - bit 12;
    uint32_t cce_pos = (ic->channel - 1) * 4;

    // setting the bit
    ic->Instance->CCER |= (1 << cce_pos);

    // set the polarity
    // channel 1 - bits 1 and 3; channel 2 - bits 5 and 7; channel 3 - bits 9 and 11; channel 4 - bits 13 and 15;

    // channel 1 - (1 - 1) * 4 + 1 = 1
    // channel 2 - (2 - 1) * 4 + 1 = 5
    // channel 3 - (3 - 1) * 4 + 1 = 9
    // channel 4 - (4 - 1) * 4 + 1 = 13
    uint32_t polarity_lsb_pos = ((ic->channel - 1) * 4 + 1); // least significant bit
    uint32_t polarity_msb_pos = polarity_lsb_pos + 2;        // most significant bit

    // setting the polarity

    // clearing the bits
    ic->Instance->CCER &= ~((1 << polarity_msb_pos) | (1 << polarity_lsb_pos));

    // since the rising edge is 00, we do not need to set any other bit after clearing

    if (ic->polarity == IC_FE)
    {
        // falling edge is 01, so we need to set the LSB to 1
        ic->Instance->CCER |= (1 << polarity_lsb_pos);
    }
    else if (ic->polarity == IC_BE)
    {
        // both edges is 11, so we need to set both bits to 1
        ic->Instance->CCER |= ((1 << polarity_msb_pos) | (1 << polarity_lsb_pos));
    }

    // enable the counter in CR1 (setting bit 0 CEN to 1)
    ic->Instance->CR1 |= (1 << 0);
}

int main(void)
{
    IC_HandleTypeDef input_capture;
    input_capture.Instance = TIM3;
    input_capture.Port = GPIOB;
    input_capture.pin = 0;
    input_capture.af = GPIO_AF2;
    input_capture.channel = 3;
    input_capture.polarity = IC_FE;

    input_capture_init(&input_capture);

    uint32_t first_timestamp = 0, second_timestamp = 0, delta = 0;

    uint32_t polarity_lsb_pos = ((input_capture.channel - 1) * 4 + 1); // least significant bit
    uint32_t polarity_msb_pos = polarity_lsb_pos + 2;                  // most significant bit

    while (1)
    {
        while (input_capture.Instance->SR & (1 << input_capture.channel))
        {
            if (first_timestamp == 0)
            {
                first_timestamp = input_capture.Instance->CCR[input_capture.channel - 1];

                // setting the polarity

                // clearing the bits
                input_capture.Instance->CCER &= ~((1 << polarity_msb_pos) | (1 << polarity_lsb_pos));

                if (input_capture.polarity == IC_FE)
                {
                    // since the rising edge is 00, we do not need to set any other bit after clearing
                    input_capture.polarity = IC_RE;
                }
                else if (input_capture.polarity == IC_RE)
                {
                    // falling edge is 01, so we need to set the LSB to 1
                    input_capture.Instance->CCER |= (1 << polarity_lsb_pos);
                    input_capture.polarity = IC_FE;
                }
            }
            else
            {
                second_timestamp = input_capture.Instance->CCR[input_capture.channel - 1];

                input_capture.Instance->CCER &= ~((1 << polarity_msb_pos) | (1 << polarity_lsb_pos));

                if (input_capture.polarity == IC_FE)
                {
                    // since the rising edge is 00, we do not need to set any other bit after clearing
                    input_capture.polarity = IC_RE;
                }
                else if (input_capture.polarity == IC_RE)
                {
                    // falling edge is 01, so we need to set the LSB to 1
                    input_capture.Instance->CCER |= (1 << polarity_lsb_pos);
                    input_capture.polarity = IC_FE;
                }
            }
        }

        if (first_timestamp && second_timestamp)
        {
            delta = second_timestamp - first_timestamp;

            first_timestamp = 0, second_timestamp = 0;
        }
    }

    return 0;
}