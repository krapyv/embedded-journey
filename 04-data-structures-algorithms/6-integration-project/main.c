#include <memory.h>
#include <stdio.h>
#include "core_cm4.h"
#include "stm32f411.h"
#include "app_config.h"
#include "systick.h"
#include "uart.h"
#include "mcp2515.h"
#include "i2c.h"
#include "bmp280.h"
#include "../../my_reusable_drivers/utils/ema/ema.h"
#include "../../my_reusable_drivers/utils/queue/queue.h"
#include "../../my_reusable_drivers/utils/pid/pid.h"
#include "../../my_reusable_drivers/utils/crc16/crc16.h"
#include "../../my_reusable_drivers/periph/encoder/encoder.h"
#include "../../my_reusable_drivers/periph/motor/motor.h"

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

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

static volatile uint32_t entry;
static volatile uint32_t last_entry = 0;
static volatile uint32_t period;
static volatile uint32_t max_period = 0;
static volatile uint32_t min_period = UINT32_MAX;
static bool have_last = false;

uint32_t can_counter = 0;
uint32_t tick_counter = 0;

Queue_t queue;
Payload_t payload_buffer[16];
EMA_t ema;
EMA_t ema_filt;
PID_t pid;

volatile bool open_loop_duty = false;

volatile float speed;
volatile float u = 0;

volatile float step_log[100];
volatile uint8_t step_log_index; // 256 samples max
volatile bool step_log_recording = false;

volatile Logger_t logger;

uint32_t last_seen = 0;
bool logger_primed = false;

uint32_t cumulative_error_counter = 0;
uint32_t error_counters[13];
BMP280_Status_t last_error_code;

uint8_t sensor_buf[22];

volatile uint16_t crc_res;

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
            message.timestamp = SysTick_GetTick();
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
            message.timestamp = SysTick_GetTick();

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
    entry = DWT->CYCCNT;

    if (have_last)
    {
        period = entry - last_entry;

        if (period < min_period)
        {
            min_period = period;
        }

        if (period > max_period)
        {
            max_period = period;
        }
    }
    last_entry = entry;
    have_last = true;

    previous_interrupt = entry;

    // clear an update interrupt flag, so the ISR won't re-enter forever
    TIM2->SR &= ~(1 << 0U);

    int16_t delta = Encoder_ReadDelta();

    tim2_counter++;

    Payload_t message = {
        .payload.tick_count = tim2_counter,
        .producer_type = Payload_Timer,
        .timestamp = SysTick_GetTick()};

    // second producer to the queue
    if (!queue_push(&queue, &message))
    {
        tim2_push_counter++;
    }

    // TIM2_counter++;
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

    logger.raw = delta;
    logger.setpoint = pid.target;
    logger.tick++;

    logger.PID_output = Motor_Duty_Direction(u);

    // NOTE: 15ms testing

    // if (TIM2_counter == 67U)
    // {
    //     elapsed_ms = SysTick_GetTick() - SYSTICK_start;
    // }

    next_interrupt = DWT->CYCCNT;
    last_delta = next_interrupt - previous_interrupt;
    if (last_delta > worst_delta)
    {
        worst_delta = last_delta;
    }
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
    TIM2->ARR = (CONTROL_TICK_MS * (TIMER_CLOCK / 1000U) - 1U); // 0.015 * 16 000 000 - 1 = 239 999

    // make sure PSC is set to 0
    TIM2->PSC = 0;

    // make sure CNT is set to 0 (due to the possible re-init or a bootloader handoff)
    TIM2->CNT = 0;

    // make sure there is no stale pending Update interrupt
    TIM2->SR &= ~(1UL << 0U);

    // ------ NVIC config ------
    // set priority to TIM2 (more urgent = smaller numerical value)
    // the core implements only the top 4 bits, so << 4 is mandatory to get it right
    NVIC->IPR[28] = ((uint8_t)TIM2_INTERRUPT_PRIORITY_LEVEL << 4U);

    // enable the NVIC for TIM2 (position 28 in the vector table)
    NVIC->ISER[0] = (1UL << 28U);
    // ------ NVIC config ------

    // enable Update interrupt
    TIM2->DIER |= (1 << 0U);

    // enable counter
    TIM2->CR1 |= (1UL << 0U);
}

void logger_snapshot()
{
    if (logger_primed && logger.tick == last_seen)
    {
        return; // discard duplicates
    }
    uint32_t seqlock_start;
    Logger_t local_logger;
    uint32_t tick_after_copy;

    do
    {
        seqlock_start = logger.tick;

        __COMPILER_BARRIER();
        local_logger.raw = logger.raw;
        local_logger.filtered = logger.filtered;
        local_logger.setpoint = logger.setpoint;
        local_logger.PID_output = logger.PID_output;
        local_logger.tick = seqlock_start;
        __COMPILER_BARRIER();

        tick_after_copy = logger.tick;
    } while (tick_after_copy != seqlock_start);

    last_seen = tick_after_copy;

    printf("%ld,%ld,%ld,%ld,%lu\n", local_logger.raw, local_logger.filtered, local_logger.setpoint, local_logger.PID_output, local_logger.tick);

    logger_primed = true;
}

static void halt_init()
{
    Motor_Disable();
    // failure branch
    while (1)
    {
    }
}

void enter_error(BMP280_HandleTypeDef *hbmp, BMP280_State_t state, BMP280_State_t current_state, BMP280_Status_t status_code)
{
    if ((state != BMP280_STATE_ERROR) && (state != BMP280_STATE_FAULT))
        return;

    cumulative_error_counter++;
    error_counters[current_state]++;
    last_error_code = status_code;
    hbmp->state = state;
}

void put_be16(uint8_t *buf, uint32_t k, uint16_t value)
{
    uint8_t high_byte = ((uint16_t)value >> 8);
    uint8_t low_byte = (value & 0xFF);

    buf[k] = high_byte;
    buf[k + 1] = low_byte;
}

void put_be32(uint8_t *buf, uint32_t k, uint32_t value)
{
    uint8_t msb = (value >> 24);
    uint8_t xmsb = (value >> 16) & 0xFF;
    uint8_t xlsb = (value >> 8) & 0xFF;
    uint8_t lsb = (value & 0xFF);

    buf[k] = msb;
    buf[k + 1] = xmsb;
    buf[k + 2] = xlsb;
    buf[k + 3] = lsb;
}

void sensor_fill(uint8_t *sensor_buf, uint32_t measure_tick, int32_t temp_value, uint32_t press_value, int32_t fast_channel, uint8_t status)
{
    put_be16(sensor_buf, 0U, RECORD_SYNC);
    sensor_buf[2] = RECORD_TYPE_SENSOR;
    put_be32(sensor_buf, 3U, measure_tick);
    put_be32(sensor_buf, 7U, (uint32_t)temp_value);
    put_be32(sensor_buf, 11U, press_value);
    put_be32(sensor_buf, 15U, (uint32_t)fast_channel);
    sensor_buf[19] = status;

    uint16_t covered_crc = crc16_table(&sensor_buf[2], 18U);

    put_be16(sensor_buf, 20U, covered_crc);
}

void main(void)
{
#if (RECORD_MODE == RECORD_SELF)
    crc_res = crc16_table("123456789", 9U);
    sensor_fill(sensor_buf, 0x01020304, -500, 1024U, 400, 0x01U);

    while (1)
    {
    }
#endif
    SCB->CPACR = (0xFU << 20U);

    __DSB();
    __ISB();

    // globally enable the DWT features
    DCB->DEMCR |= (1UL << 24U);
    // enable CYCNT
    DWT->CTRL |= (1UL << 0U);
    MCP2515_Register_Callback(Override_EXTI15_10_IRQHandler);

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    usart2_init();
    spi_init(SPI_BR_8);
    Encoder_Init();

    Motor_Init();

    if (!queue_init(&queue, payload_buffer, sizeof(Payload_t), 16U))
    {
        printf("Queue init failed!\n");
        halt_init();
    }

    if (!PID_Init(&pid, 80.0f, KP, KI, KD, CONTROL_TICK_S, 0.0f, 1.0f))
    {
        printf("PID init failed!\n");
        halt_init();
    }

#if (FILTER_MODE == UNFILTERED_BASELINE)
    if (!EMA_Init(&ema, 1U, 4U))
    {
        printf("Unfiltered EMA init failed!\n");
        halt_init();
    }
#elif (FILTER_MODE == EMA_IN_LOOP_FILTER_MODE)
    if (!EMA_Init(&ema_filt, 1U, 4U))
    {
        printf("In the loop EMA init failed!\n");
        halt_init();
    }
#elif (FILTER_MODE == MEDIAN_EMA_IN_LOOP_FILTER_MODE)
#error "Unimplemented MEDIAN_EMA_IN_LOOP_FILTER_MODE"
#elif (FILTER_MODE == EMA_OUTSIDE_LOOP_FILTER_MODE)
#error "Unimplemented EMA_OUTSIDE_LOOP_FILTER_MODE"
#else
#error "Unknown filter mode"
#endif

    // set the EXTI15_10 a priority of 8
    NVIC->IPR[40] = ((uint8_t)MCP2515_INTERRUPT_PRIORITY_LEVEL << 4U);

    // enable the NVIC for EXTI15_10 (position 40 in the vector table)
    // 40 / 32 = 1, 40 % 32 = 8
    // NVIC->ISER[1] = (1UL << 8U);

    // set priority to SysTick (less urgent = bigger numerical value)
    // also implemented only the top 4 bits, so << 4 is mandatory as well
    // bits 31:28

    // clear the range
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    SCB->SHPR3 &= ~(0xFU << 28U);

    // set to the priority of 2 (0x2)
    SCB->SHPR3 |= (0x2 << 28U);

    mcp2515_reset();

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

    // NVIC->ICER[1] = (1UL << 8U);
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

    // BMP280

    // define and initialize structs
    hi2c.channel = I2C_CHANNEL_1;
    hi2c.scl_port = GPIOB;
    hi2c.scl_pin = 6;
    hi2c.sda_port = GPIOB;
    hi2c.sda_pin = 7;
    hi2c.state = I2C_STATE_IDLE;
    hi2c.sb_hits = 0;
    hi2c.stop_hits = 0;
    hi2c.start_pending_hits = 0;

    BMP280_HandleTypeDef hbmp = {
        .hi2c = &hi2c,
        .slave_addr = BMP280_I2C_ADDR,
        .isInitialized = 0};

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    // peripherals init
    I2C_Init();

    hbmp.state = BMP280_STATE_INIT;

    // BMP280

    while (1)
    {
        logger_snapshot();
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

        I2C_Process();

        BMP280_Status_t status_res;
        switch (hbmp.state)
        {
        case BMP280_STATE_IDLE:
            break;
        case BMP280_STATE_INIT:
            status_res = BMP280_Init(&hbmp, meas);

            if (status_res == BMP280_ERR_CHIP_ID)
            {
                // wrong id = wrong device
                enter_error(&hbmp, BMP280_STATE_FAULT, hbmp.state, status_res);
            }
            else if (status_res != BMP280_OK)
            {
                enter_error(&hbmp, BMP280_STATE_ERROR, hbmp.state, status_res);
            }
            break;
        case BMP280_STATE_READ_CALIBRATION:
            status_res = BMP280_ReadCalibration(&hbmp);
            if (status_res != BMP280_OK)
            {
                enter_error(&hbmp, BMP280_STATE_ERROR, hbmp.state, status_res);
            }
            break;
        case BMP280_STATE_RECONSTRUCT_CALIBRATION:
            BMP280_ReconstructCalibration(&hbmp);
            break;
        case BMP280_STATE_CTRL_MEAS:
            status_res = BMP280_WriteCtrlMeas(&hbmp);
            if (status_res != BMP280_OK)
            {
                enter_error(&hbmp, BMP280_STATE_ERROR, hbmp.state, status_res);
            }
            break;
        case BMP280_STATE_MEASURING:
            status_res = BMP280_Measuring(&hbmp);
            if (status_res != BMP280_OK)
            {
                enter_error(&hbmp, BMP280_STATE_ERROR, hbmp.state, status_res);
            }
            break;
        case BMP280_STATE_READ_MEASURAMENTS:
            status_res = BMP280_ReadMeasurements(&hbmp);
            if (status_res != BMP280_OK)
            {
                enter_error(&hbmp, BMP280_STATE_ERROR, hbmp.state, status_res);
            }
            break;
        case BMP280_STATE_RECONSTRUCT_MEASURAMENTS:
            BMP280_ReconstructMeasurements(&hbmp);
            break;
        case BMP280_STATE_COMPENSATE:
            BMP280_CalculateData(&hbmp);
            break;
        case BMP280_STATE_READY:
            // printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", hbmp.temp_value / 100, hbmp.press_value / 256 / 100);
            // fflush(stdout);

            // printf("start_pending_hits: %d | sb_hits: %d\r\n", hi2c.start_pending_hits, hi2c.sb_hits);

            hbmp.request_status = BMP280_REQUEST_NONE;
            hbmp.retries = 0;
            hbmp.measure_start_tick = 0;
            hbmp.measure_start_tick_status = BMP280_START_TICK_NEVER_CAPTURED;
            // BMP start measurements
            hbmp.state = BMP280_STATE_CTRL_MEAS;

            break;
        case BMP280_STATE_ERROR:
            if (hbmp.retries >= 3)
            {
                // the counter is exhausted
                hbmp.state = BMP280_STATE_FAULT;
                break;
            }

            if (hbmp.hi2c->state == I2C_STATE_IDLE)
            {
                hbmp.retries++;

                if (hbmp.isInitialized == 0)
                {
                    hbmp.state = BMP280_STATE_INIT;
                }
                else
                {
                    // begin a transaction from the beginning (Calibration is read once at the very beginning, so omit the state)
                    hbmp.state = BMP280_STATE_CTRL_MEAS;
                }
            }

            break;
        case BMP280_STATE_FAULT:
            printf("The BMP280 sensor experienced hard fault!\n");
            hbmp.state = BMP280_STATE_DEAD;
            break;

        case BMP280_STATE_DEAD:
            break;
        }
    }
}