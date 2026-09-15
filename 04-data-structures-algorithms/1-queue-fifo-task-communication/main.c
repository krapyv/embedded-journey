#include <memory.h>
#include <inttypes.h>
#include "app_config.h"
#include "mcp2515.h"
#include "systick.h"
#include "uart.h"

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

volatile uint32_t tim2_counter = 0;

Queue_t queue;
Payload_t payload_buffer[16];

bool queue_init(Queue_t *queue, Payload_t *storage, uint8_t size)
{
    bool is_power_of_two = (size & (size - 1)) == 0;
    if (size <= 0 || !is_power_of_two)
    {
        return false;
    }

    queue->buffer = storage;
    queue->head = 0;
    queue->tail = 0;
    queue->max_size = size;

    return true;
}

bool queue_push(Queue_t *queue, Payload_t message)
{
    if (queue_is_full(queue))
    {
        return false;
    }

    queue->buffer[queue->head] = message;

    queue->head = ((queue->head + 1) & (queue->max_size - 1));

    return true;
}

bool queue_pop(Queue_t *queue, Payload_t *message)
{
    if (queue_is_empty(queue))
    {
        return false;
    }

    *message = queue->buffer[queue->tail];

    queue->tail = ((queue->tail + 1) & (queue->max_size - 1));

    return true;
}

bool queue_is_empty(Queue_t *queue)
{
    return queue->head == queue->tail;
}

bool queue_is_full(Queue_t *queue)
{
    return ((queue->head + 1) & (queue->max_size - 1)) == queue->tail;
}

void Override_EXTI15_10_IRQHandler(void)
{
    // the MCP2515 CAN interrupt can be triggered once the message was received in RXB0 or RXB1
    // the CANable connected to the laptop acts as the second CAN node which sends the frames

    // first of all, we need to know which exactly EXTI line caused the ISR to enter
    // EXTI Line 15 corresponds with EXIT_PR bit 15 set to 1

    if (EXTI->PR & (1 << 15))
    {
        // clear the bit 15 in EXTI_PR
        EXTI->PR = (1 << 15); // write-1-to-clear
        // set the flag to 1
        // INT line dropped to 0 and caused the interrupt

        uint8_t can_int_rx0_header[5];
        uint8_t can_int_rx0_payload[8];
        uint8_t can_int_rx0_flag = 0;

        uint8_t can_int_rx1_header[5];
        uint8_t can_int_rx1_payload[8];
        uint8_t can_int_rx1_flag = 0;

        uint8_t can_intf_val;

        do
        {

            mcp2515_read(CANINTF, &can_intf_val, 1U);

            if (can_intf_val)
            {
                mcp2515_canintf_handler(can_intf_val, can_int_rx0_header, can_int_rx0_payload, &can_int_rx0_flag, can_int_rx1_header, can_int_rx1_payload, &can_int_rx1_flag);
            }
        } while (can_intf_val & ~((1 << 7) | (1 << 5)));

        Payload_t message;
        message.producer_type = Payload_MCP2515;

        CAN_Frame_t frame;

        if (can_int_rx0_flag)
        {
            frame.SIDH = can_int_rx0_header[0];
            frame.SIDL = can_int_rx0_header[1];
            frame.EID8 = can_int_rx0_header[2];
            frame.EID0 = can_int_rx0_header[3];
            frame.DLC = can_int_rx0_header[4];

            memcpy(frame.data, can_int_rx0_payload, 8 * sizeof(uint8_t));

            message.payload.mcp2515_frame = frame;
            queue_push(&queue, message);

            can_int_rx0_flag = 0;
        }
        // if rx1_flag is set, the rx1_buffer has a new frame
        if (can_int_rx1_flag)
        {
            frame.SIDH = can_int_rx1_header[0];
            frame.SIDL = can_int_rx1_header[1];
            frame.EID8 = can_int_rx1_header[2];
            frame.EID0 = can_int_rx1_header[3];
            frame.DLC = can_int_rx1_header[4];

            memcpy(frame.data, can_int_rx1_payload, 8 * sizeof(uint8_t));

            message.payload.mcp2515_frame = frame;
            queue_push(&queue, message);

            can_int_rx1_flag = 0;
        }
    }

    return;
}

void TIM2_init(void)
{
    // let's assume that a timer will produce an interrupt once in 20 ms

    // enable RCC TIM2 clock
    RCC->APB1ENR |= (1UL << 0U);

    // ensure the CMS bits (6:5) in CR1 are 00 - edge-aligned mode, so the counter counts up or down depending on the direction bit (DIR)
    // 11 = 0x3
    TIM2->CR1 &= ~(0x3UL << 5U);

    // set the downcounter direction: bit 4 DIR to 1
    TIM2->CR1 |= (1UL << 4U);

    // ensure the UDIS (bit 1) is 0, so UEV is enabled
    TIM2->CR1 &= ~(1UL << 1U);

    // set the URS (bit 2) - Update request source - to 1
    // so only counter overflow/underflow generates an update interrupt
    TIM2->CR1 |= (1UL << 2U);

    // set UIE (bit 0) of DIER to 1 to enable Update interrupts
    TIM2->DIER |= (1UL << 0U);

    // set auto-reload register
    // HSI oscillator has by default 16MHz frequency
    // 1 ms = 16 000 ticks. then 20 ms = 20 * 16 000 = 320 000
    // ARR = 319 999 - since TIM2 has a 32-bit ADD register, it is ok
    TIM2->ARR = 319999;

    // TIM2 Handler has the position 28 in the vector table
    // the position 28 is the ISER0 bit 28
    // set the TIM2 a priority of 4
    NVIC->IPR[28] = ((uint8_t)TIM2_INTERRUPT_PRIORITY_LEVEL << 4U);

    NVIC->ISER[0] = (1UL << 28U);

    // enable counter
    TIM2->CR1 |= (1UL << 0U);
}

void TIM2_IRQHandler(void)
{
    tim2_counter++;

    // clear UIF bit 0 of TIM2->SR
    // UIF is write-0-to-clear
    TIM2->SR &= ~(1UL << 0U);

    Payload_t message = {
        .payload.tick_count = tim2_counter,
        .producer_type = Payload_Timer};

    uint8_t current_basepri = __get_basepri();
    __set_basepri(MCP2515_INTERRUPT_PRIORITY_LEVEL);

    bool result = queue_push(&queue, message);

    __set_basepri(current_basepri);
}

void main(void)
{
    MCP2515_Register_Callback(Override_EXTI15_10_IRQHandler);

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    spi_init(SPI_BR_8);

    queue_init(&queue, payload_buffer, 16U);

    // set the EXTI15_10 a priority of 4
    NVIC->IPR[40] = ((uint8_t)MCP2515_INTERRUPT_PRIORITY_LEVEL << 4U);

    mcp2515_reset();

    usart2_init();

    uint8_t isSuccess;

    // CNF3, CNF2, CNF1
    uint8_t CNF_vals[3] = {0x01, 0x91, 0x00};

    // CNF3 = 0x28, CNF2 = 0x29, CNF1 = 0x2A
    // since all three registers are consecutive, we can write them in one write operation
    mcp2515_write(CNF3, CNF_vals, 3U);

    uint8_t read_CNF_vals[3];

    mcp2515_read(CNF3, read_CNF_vals, 3U);

    uint8_t CANCTRL_val;

    mcp2515_read(CANCTRL1, &CANCTRL_val, 1);

    mcp2515_init();

    // enable interrupts via MCP2515 CANINTE register
    // bit 5 ERRIE, bit 1 RX1IE, bit 0 RX0IE

    // mask: 0010 0011 = 2^5 + 2^1 + 2^0 = 32 + 2 + 1 = 35 = 0x23
    // data byte: 0010 0011 = 0x23

    mcp2515_bit_modify(CANINTE, 0x23, 0x23);

    // request Normal mode (bits 7-5: 000)

    // 0b1110 0000 = 0xE0 <- mask, bits 7-5 are requested to change
    // 0b0000 0000 = 0x00 <- data byte, bits 7-5 are requested to changed to 000
    mcp2515_bit_modify(CANCTRL1, 0xE0, 0x00);

    // waiting loop for mode to change
    // mask: 0b1110 0000 = 0xE0 - bits 7-5 are relevant only

    mcp2515_poll_bit_timeout(CANSTAT1, 0xE0, 0x00, 10U, &isSuccess);

    TIM2_init();

    Payload_t message;

    while (1)
    {
        if (queue_pop(&queue, &message))
        {
            if (message.producer_type == Payload_MCP2515)
            {
                // CAN frame has a 11-bit wide ID
                // SIDH contains bits 11:3, SIDL - bits 2:0 of that 11 bit ID
                // SIDH has all 8 bits with ID data, SIDL - bits 7:5
                uint16_t id = message.payload.mcp2515_frame.SIDH | (message.payload.mcp2515_frame.SIDL >> 5U);

                // DLC register has DLC bits 3:0
                // 111 = 0x7
                uint8_t dlc = message.payload.mcp2515_frame.DLC & 0x7;

                printf("ID: %" PRIu16 " | DLC: %" PRIu8 " | Payload: ", id, dlc);

                for (uint8_t i = 0; i < dlc; i++)
                {
                    printf("%02" PRIx8 " ", message.payload.mcp2515_frame.data[i]);
                }

                printf("\r\n");
            }
            else
            {
                printf("Tick counter: %" PRIu32 "\r\n", message.payload.tick_count);
            }
        }
    }
}