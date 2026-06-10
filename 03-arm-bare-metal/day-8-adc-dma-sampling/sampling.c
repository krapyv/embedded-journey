#include "core_cm4.h"
#include "stm32f411.h"
#include "sampling.h"

static volatile uint16_t adc_raw_buffer[ADC_BUFFER_SIZE];

void DMA2_Stream0_IRQHandler(void)
{
    // TCIF: transfer complete interrupt, stream 0 - bit 5
    if (DMA2->LISR & (1UL << 5))
    {
        // clearing the TCIF
        DMA2->LIFCR = (1UL << 5);
    }

    // HTIF: hakf transfer interrupt, stream 0 - bit 4
    if (DMA2->LISR & (1UL << 4))
    {

        // clearing the HTIF
        DMA2->LIFCR = (1UL << 4);
    }
}

void clocks_init(void)
{
// ---- PORT CLOCK ENABLING

// since only ports A and B can be ADC
#if (TARGET_SAMP_PORT == HARDWARE_PORT_A)
    RCC->AHB1ENR |= (1UL << 0U);

#elif (TARGET_SAMP_PORT == HARDWARE_PORT_B)
    RCC->AHB1ENR |= (1UL << 1U);
#else
#error "Invalid target port assignment! Must use HARDWARE_PORT_A or HARDWARE_PORT_B"
#endif

    // ---- DMA ENABLING ----

    // DMA2 (bit 22) for reading ADC data
    // DMA1 (bit 21) for USART2 communication

    RCC->AHB1ENR |= 1UL << 22U;
    RCC->AHB1ENR |= 1UL << 21U;

    // ---- ADC1 ENABLING ----
    RCC->APB2ENR |= 1UL << 8U;

    // ---- TIM2 ENABLING ----
    RCC->APB1ENR |= 1UL << 0U;

    // ---- UART2 ENABLING ----
    RCC->APB1ENR |= 1UL << 17U;

#if (TARGET_SAMP_PORT == HARDWARE_PORT_A)
// Port A has already been enabled
#elif (TARGET_SAMP_PORT == HARDWARE_PORT_B)
    /* Port A enabling for USART */
    RCC->AHB1ENR |= 1UL << 0U;
#endif
}

void gpio_init(void)
{
// ---- POTENTIOMETER PIN CONFIGURATION ----
#if (TARGET_SAMP_PORT == HARDWARE_PORT_A)
    // a pin has 2 bits in MODER
    // Analog mode - 11

    // 0x3 = 0b11
    GPIOA->MODER &= ~(0x3UL << (TARGET_ADC_PIN * 2));
    GPIOA->MODER |= (0x3UL << (TARGET_ADC_PIN * 2));
#elif (TARGET_SAMP_PORT == HARDWARE_PORT_B)
    GPIOB->MODER &= ~(0x3UL << (TARGET_ADC_PIN * 2));
    GPIOB->MODER |= (0x3UL << (TARGET_ADC_PIN * 2));
#endif

    // ---- UART2 TX PIN PA2 CONFIGURATION ----
    // Alternate Function - 10
    GPIOA->MODER &= ~(0x3UL << 4U); // pin PA2 takes bits 5:4

    // 0x2 = 0b10
    GPIOA->MODER |= (0x2UL << 4U);

    // USART1/2 work with AF7 (0111)

    // AFRL since the PA2
    // PA2 has bits 11:8

    // to clear, use 0xF = 0b1111
    GPIOA->AFRL &= ~(0xFUL << (4 * 2));

    // 0x7 = 0b0111
    GPIOA->AFRL |= (0x7UL << (4 * 2));
}

void tim_init(void)
{
    TIM2->PSC = 15U;
    TIM2->ARR = 99U;

    // 0b111 = 2^2 + 2^1 + 2^0 = 7 = 0x7
    // MMS - bits 6:4
    TIM2->CR2 &= ~(0x7UL << 4U);

    // Update: 010 = 0x2
    TIM2->CR2 |= (0x2UL << 4U);
}

void dma_init(void)
{
    // ---- DMA2 CONFIGURATION ----

    // DMA2 Stream 0 Channel 0

    // channel selection

    // 0b111 = 0x7
    DMA2->S0CR &= ~(0x7UL << 25U);
    // since channel 0 is 000, only clearing is required

    // MSIZE: bits 14:13
    // 01 - half-word (16-bit)
    DMA2->S0CR &= ~(0x3UL << 13U);

    // 0x1 = 0b01
    DMA2->S0CR |= (0x1UL << 13U);

    // PSIZE: bits 12:11
    // 01 - half-word (16-bit)
    DMA2->S0CR &= ~(0x3UL << 11U);
    DMA2->S0CR |= (0x1UL << 11U);

    // MINC: bit 10
    DMA2->S0CR |= (1UL << 10U);

    // PINC: bit 9 (set to 0)
    DMA2->S0CR &= ~(1UL << 9U);

    // CIRC: bit 8
    DMA2->S0CR |= (1UL << 8U);

    // DIR: bits 7:6
    // Peripheral-to-memory: 00

    // 0b11 = 0x3
    DMA2->S0CR &= ~(0x3UL << 6U);

    // TCIE: bit 4
    DMA2->S0CR |= (1UL << 4U);

    // HTIE: bit 3
    DMA2->S0CR |= (1UL << 3U);

    // set DMA_PAR (peripheral address)
    DMA2->S0PAR = &ADC1->DR;

    // set DMA_M0AR (declared buffer)
    DMA2->S0M0AR = &adc_raw_buffer;

    // set DMA_NDTR (number of data items/buffer length)
    DMA2->S0NDTR = ADC_BUFFER_SIZE;

    // ---- DMA1 CONFIGURATION ----
    // DMA1 Stream 6 Channel 4 uses USART2_TX

    // channel selection (CHSEL 27:25)
    // 100 - channel 4

    // 0x7 = 0b111
    DMA1->S6CR &= ~(0x7UL << 25U);

    // 0x4 = 0b100
    DMA1->S6CR |= (0x4UL << 25U);

    // DIR: bits 7:6
    // memory-to-peripheral (because from ADC_BUFFER to USART2 PERIPHERAL)
    // 01 or 0x1U or 1U - memory-to-peripheral

    DMA1->S6CR &= ~(0x3UL << 6U);
    DMA1->S6CR |= (1UL << 6U);
}

void nvic_init(void)
{
    // DMA2_Stream0 has the position 56 in the vector table of interrupts
    // 56 / 32 = 1, 56 % 32 = 24. ISER[1] bit 24
    NVIC->ISER[1] = (1UL << 24U);
}

void adc_init(void)
{
    // ---- ADC_CR2 CONFIGURATION ----

    // SWSTART: bit 30 (set to 0)
    ADC1->CR2 &= ~(1UL << 30U);

    // EXTEN: bits 29:28
    // 01 - trigger detection on the rising edge
    // 0x3 = 0b11, 0x1 = 1 = 0x01

    ADC1->CR2 &= ~(0x3UL << 28U);
    ADC1->CR2 |= (1UL << 28U);

    // EXTSEL (External event select for regular group): bits 27:24
    // 0110 - Timer 2 TRGO event

    // 0b1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    ADC1->CR2 &= ~(0xFUL << 24U);

    // 0b0110 = 2^2 + 2^1 = 6 = 0x6
    ADC1->CR2 |= (0x6UL << 24U);

    // ALIGN : bit 11 (set to 0)
    ADC1->CR2 &= ~(1UL << 11U);

    // DDS : bit 9
    ADC1->CR2 |= (1UL << 9U);

    // DMA : bit 8
    ADC1->CR2 |= (1UL << 8U);

    // CONT: bit 1 (set to 0)
    ADC1->CR2 &= (1UL << 1U);

    // ---- ADC_CCR CONFIGURATION ----
    // ADCPRE: bit 17:16
    // 0x3 = 0b11
    ADC->CommonRegisters.CCR &= ~(0x3UL << 16U);
    ADC->CommonRegisters.CCR |= (ADC_PRESCALER << 16U);

    // ---- ADC_SMPR2 CONFIGURATION ----

#if (TARGET_ADC_CHANNEL <= 9)
    // ADC_SMPR2 handles channels 0 to 9

    // each channel has 3 bits inside the register

    // 0b111 = 0x7
    ADC1->SMPR2 &= ~(0x7UL << (TARGET_ADC_CHANNEL * 3));
    ADC1->SMPR2 |= (ADC_SAMPLE_CYCLES << (TARGET_ADC_CHANNEL * 3));
#else
#error "Invalid target ADC channel assignment! Must use channels 0 to 9."
#endif

    // ---- CONVERTER ENABLING ----
    // ADC_CR2
    // ADON: bit 0
    ADC1->CR2 |= (1UL << 0U);
}

void uart_init(void)
{
    // pretty much full initialization is handled by the UART custom driver

    // ---- CR3 CONFIGURATION ----
    // DMAT (DMA enable transmitter): bit 7
    USART2->CR[2] |= (1UL << 7U);
}

void pipeline_enabling(void)
{
    // DMA2 Stream 0 enabling
    // EN: bit 0
    DMA2->S0CR |= (1UL << 0U);

    // DMA1 Stream 6 enabling
    DMA1->S6CR |= (1UL << 0U);

    // TIM2 enabling
    // CEN: bit 0
    TIM2->CR1 |= (1UL << 0U);
}