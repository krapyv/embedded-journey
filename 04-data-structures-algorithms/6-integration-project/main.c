#include <memory.h>
#include <stdio.h>
#include "core_cm4.h"
#include "stm32f411.h"
#include "app_config.h"
#include "systick.h"
#include "uart.h"
#include "mcp2515.h"
#include "../../my_reusable_drivers/utils/ema/ema.h"
#include "../../my_reusable_drivers/utils/queue/queue.h"
#include "../../my_reusable_drivers/utils/pid/pid.h"
#include "../../my_reusable_drivers/periph/encoder/encoder.h"
#include "../../my_reusable_drivers/periph/motor/motor.h"

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

// verification variables
static volatile uint32_t previous_interrupt;
static volatile uint32_t next_interrupt;
static volatile uint32_t last_delta;
static volatile uint32_t worst_delta;

static volatile uint32_t can_previous_interrupt;
static volatile uint32_t can_next_interrupt;
static volatile uint32_t can_last_delta;
static volatile uint32_t can_worst_delta;

volatile uint8_t interrupts_count = 0;

volatile uint32_t tim2_counter = 0;

static volatile uint32_t tim2_push_counter = 0;
static volatile uint32_t can_push_counter = 0;

uint32_t can_counter = 0;
uint32_t tick_counter = 0;

Queue_t queue;
Payload_t payload_buffer[16];
EMA_t ema;
EMA_t ema_filt;
PID_t pid;

volatile uint16_t prev_count;

volatile bool open_loop_duty = false;

volatile float speed;
volatile float K = 160; // measured at 6 V
volatile float u = 0;
volatile float target = 80;

volatile float step_log[100];
volatile uint8_t step_log_index; // 256 samples max
volatile bool step_log_recording = false;

volatile Logger_t logger;

void Override_EXTI15_10_IRQHandler(void)
{
    can_previous_interrupt = DWT->CYCCNT;
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
            uint8_t current_basepri = __get_basepri();

            // blocks priority numbers >= 1
            __set_basepri(TIM2_INTERRUPT_PRIORITY_LEVEL);
            if (!queue_push(&queue, &message))
            {
                can_push_counter++;
            }
            __set_basepri(current_basepri);
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

            uint8_t current_basepri = __get_basepri();

            // blocks priority numbers >= 1
            __set_basepri(TIM2_INTERRUPT_PRIORITY_LEVEL);
            if (!queue_push(&queue, &message))
            {
                can_push_counter++;
            }
            __set_basepri(current_basepri);
            can_int_rx1_flag = 0;
        }
    }

    can_next_interrupt = DWT->CYCCNT;
    can_last_delta = can_next_interrupt - can_previous_interrupt;
    if (can_last_delta > can_worst_delta)
    {
        can_worst_delta = can_last_delta;
    }

    return;
}

void TIM2_IRQHandler(void)
{
    // clear an update interrupt flag, so the ISR won't re-enter forever
    TIM2->SR &= ~(1 << 0U);

    // TIM2_counter++;
    uint16_t current_cnt = TIM1->CNT;

    int16_t delta = (uint16_t)(current_cnt - prev_count);
    int32_t acc = EMA_Update(&ema_filt, delta);

    (void)acc;
// speed += ((float)CONTROL_TICK_MS / (float)TIME_CONSTANT) * (K * u - speed);
#if (FILTER_MODE == UNFILTERED_BASELINE)
    speed = (float)delta / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0f;
    logger.filtered = delta * (1 << (ema.k + ema.frac));
#elif (FILTER_MODE == EMA_IN_LOOP_FILTER_MODE)
    speed = (float)acc / (1 << (ema_filt.k + ema_filt.frac)) / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0f;
    logger.filtered = acc;
#elif (FILTER_MODE == MEDIAN_EMA_IN_LOOP_FILTER_MODE)
#error "Unimplemented MEDIAN_EMA_IN_LOOP_FILTER_MODE"
#elif (FILTER_MODE == EMA_OUTSIDE_LOOP_FILTER_MODE)
#error "Unimplemented EMA_OUTSIDE_LOOP_FILTER_MODE"
#else
#error "Unknown filter mode"
#endif

    if (step_log_recording)
    {

        if (step_log_index < 100)
        {
            step_log[step_log_index] = speed;
            step_log_index++;
        }
        else
        {
            step_log_recording = false;
        }
    }

    if (!open_loop_duty)
    {
        u = PID_Update(&pid, speed);
    }
    prev_count = current_cnt;

    logger.raw = delta;
    logger.setpoint = (int32_t)target;
    logger.tick++;

    logger.PID_output = Motor_Duty_Direction(u);

    // NOTE: 15ms testing

    // if (TIM2_counter == 67U)
    // {
    //     elapsed_ms = SysTick_GetTick() - SYSTICK_start;
    // }
}

void TIM2_init(void)
{
    // a timer needs to produce an interrupt once in 15 ms

    // enable RCC TIM2 clock
    RCC->APB1ENR |= (1UL << 0U);

    // ensure the CMS bits (6:5) in CR1 are 00 - edge-aligned mode, so the counter counts up or down depending on the direction bit (DIR)
    // 11 = 0x3
    TIM2->CR1 &= ~(0x3UL << 5U);

    // set the upcounting direction: bit 4 DIR to 0
    TIM2->CR1 &= ~(1UL << 4U);

    // ensure the UDIS (bit 1) is 0, so UEV is enabled
    TIM2->CR1 &= ~(1UL << 1U);

    // set the URS (bit 2) - Update request source - to 1
    // so only counter overflow/underflow generates an update interrupt
    TIM2->CR1 |= (1UL << 2U);

    // set UIE (bit 0) of DIER to 1 to enable Update interrupts
    TIM2->DIER |= (1UL << 0U);

    // set auto-reload register
    // HSI oscillator has by default 16MHz frequency
    // 1 ms = 240 000 ticks. then 15 ms = 15 * 16 000 = 240 000
    // ARR = 239 999 - since TIM2 has a 32-bit ADD register, it is ok
    TIM2->ARR = 239999;

    // make sure PSC is set to 0
    TIM2->PSC = 0;

    // make sure CNT is set to 0 (due to the possible re-init or a bootloader handoff)
    TIM2->CNT = 0;

    // make sure there is no stale pending Update interrupt
    TIM2->SR &= ~(1UL << 0U);

    // TIM2 Handler has the position 28 in the vector table
    // the position 28 is the ISER0 bit 28
    // set the TIM2 a priority of 8
    NVIC->IPR[28] = ((uint8_t)TIM2_INTERRUPT_PRIORITY_LEVEL << 4U);

    NVIC->ISER[0] = (1UL << 28U);

    // enable counter
    TIM2->CR1 |= (1UL << 0U);
}

void main()
{
    // globally enable the DWT features
    DCB->DEMCR |= (1UL << 24U);
    // enable CYCNT
    DWT->CTRL |= (1UL << 0U);
    MCP2515_Register_Callback(Override_EXTI15_10_IRQHandler);

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    spi_init(SPI_BR_8);

    queue_init(&queue, payload_buffer, sizeof(Payload_t), 16U);
    Encoder_Init();
    Motor_Init();
    PID_Init(&pid, target, KP, KI, KD, CONTROL_TICK_S, 0.0f, 1.0f);

#if (FILTER_MODE == UNFILTERED_BASELINE)
    EMA_Init(&ema, 1U, 4U);
#elif (FILTER_MODE == EMA_IN_LOOP_FILTER_MODE)
    EMA_Init(&ema_filt, 1U, 4U);
#elif (FILTER_MODE == MEDIAN_EMA_IN_LOOP_FILTER_MODE)
#error "Unimplemented MEDIAN_EMA_IN_LOOP_FILTER_MODE"
#elif (FILTER_MODE == EMA_OUTSIDE_LOOP_FILTER_MODE)
#error "Unimplemented EMA_OUTSIDE_LOOP_FILTER_MODE"
#else
#error "Unknown filter mode"
#endif

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
        if (can_counter < 24000)
        {

            if (queue_pop(&queue, &message))
            {
                if (message.producer_type == Payload_MCP2515)
                {
                    can_counter++;
                    //     // CAN frame has a 11-bit wide ID
                    //     // SIDH contains bits 11:3, SIDL - bits 2:0 of that 11 bit ID
                    //     // SIDH has all 8 bits with ID data, SIDL - bits 7:5
                    //     uint16_t id = (message.payload.mcp2515_frame.SIDH << 3) | (message.payload.mcp2515_frame.SIDL >> 5U);

                    //     // DLC register has DLC bits 3:0
                    //     // 1111 = 8 + 4 + 2 + 1 = 15 = 0xF
                    //     uint8_t dlc = message.payload.mcp2515_frame.DLC & 0xF;

                    //     printf("ID: %u | DLC: %u | Payload: ", id, dlc);

                    //     for (uint8_t i = 0; i < dlc; i++)
                    //     {
                    //         // printf("%02x ", message.payload.mcp2515_frame.data[i]);
                    //     }

                    //     printf("\r\n");
                }
                else
                {
                    tick_counter++;
                    //     // printf("Tick counter: %u\r\n", message.payload.tick_count);
                }
            }
        }
    }
}