#include "spi.h"
#include "mcp2515.h"
#include "systick.h"
#include "led.h"

volatile uint8_t can_int_flag = 0;

// the Loopback test harness + transmission TX real CAN bus test
/*int main(void)
{
    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    spi_init(SPI_BR_8);

    // error led initialization
    LED_HandleTypeDef ledHandle = {
        .pin = 0,
        .rcc_bit = 0,
        .moder_reg = &(GPIOA->MODER),
        .odr_reg = &(GPIOA->ODR),
        .rcc_clk_reg = &(RCC->AHB1ENR)};

    led_init(&ledHandle);
    // error led initialization

    uint8_t isSuccess;

    mcp2515_reset();

    uint8_t rx_byte = 0;

    mcp2515_read(CANSTAT1, &rx_byte, 1U);

    // CNF3, CNF2, CNF1
    uint8_t CNF_vals[3] = {0x01, 0x91, 0x00};

    // CNF3 = 0x28, CNF2 = 0x29, CNF1 = 0x2A
    // since all three registers are consecutive, we can write them in one write operation
    mcp2515_write(CNF3, CNF_vals, 3U);

    uint8_t read_CNF_vals[3];

    mcp2515_read(CNF3, read_CNF_vals, 3U);

    uint8_t CANCTRL_val;

    mcp2515_read(CANCTRL1, &CANCTRL_val, 1);

    // (0x7 << 5) = 111 << 5 - bits 7-5 are about to change
    // mask: 1110 0000
    // current value: 1000 xxxx
    // data byte: MCP_Loopback_Mode (0x2 << 5) or (010 << 5)
    uint8_t mask = (0x7 << 5);
    uint8_t data_byte = MCP_Loopback_Mode;

    mcp2515_bit_modify(CANCTRL1, mask, data_byte);

    // LOOPBACK TEST BENCH

    // test id
    // since it is 11-bit long, uint16_t is the type that can hold the value without truncation
    // bits 15-12 are zeroed out
    uint16_t id = 0x1F2;

    uint8_t SIDH = id >> 3;

    // EXIDE and EID bits left at 0 for standard frames
    uint8_t SIDL = (id & 0x7) << 5;

    uint8_t bytes_number = 5;

    // bit 7: unimplemented, 0
    // bit 6: RTR, in this case for a data frame, 0
    // bits 5-4: unimplemented, 0
    // bits 3-0: number of data bytes to be transmitted (0-8 bytes)
    // 5 bytes = 0b0101 => in 8 bit representation: 0000 0101 (RTR explicitly cleared with 0)
    uint8_t DLC = bytes_number;

    // SIDH, SIDL, EID8 (zeroed out, don't care), EID0 (zeroed out, don't care), DLC, 5 data bytes
    uint8_t data_payload[10] = {SIDH, SIDL, 0x00, 0x00, DLC, 0x3F, 0xF1, 0xB4, 0x11, 0xAA};
    uint8_t load_tx_buffer_length = 10;

    mcp2515_load_tx_buffer(MCP_Load_TXB0SIDH, data_payload, load_tx_buffer_length);

    MCP_RTS_locations_t location = MCP_RTS_TXB0;

    mcp2515_rts(&location, 1U);

    // poll the RX0IF in READ_STATUS
    // RX0IF - receive buffer 0 Full Interrupt Flag bit
    // when RX0IF is 1 - interrupt is pending (must be cleared by MCU to reset the interrupt condition)
    uint8_t status_val = 0;

    do
    {
        mcp2515_read_status(&status_val, 1U);
    } while (!(status_val & (1 << 0)));

    // once RX buffer has the data frame, retrieve it
    uint8_t rx_frame_bytes[10];

    mcp2515_read_rx_buffer(MCP_Read_RXB0SIDH, rx_frame_bytes, 10U);
    // the READ RX BUFFER instruction automatically clears the associated receive flag, RXnIF (CANINTF), when CS is raised at the end of the command
    // so the RX0IF flag will be cleared automatically, so the hardware condition to reset the interrupt condition is satisfied

    uint8_t comparison_results[10];

    for (uint8_t i = 0; i < 10; i++)
    {
        // special case for SIDL since the datasheet does not guarantee the EID bits read as a defined value
        if (i == 1)
        {
            // 0x7 = 0111 (bits 7-5)
            uint8_t masked_rx_sidl = rx_frame_bytes[i] & (0x7 << 5);
            uint8_t masked_tx_sidl = data_payload[i] & (0x7 << 5);

            if (masked_rx_sidl == masked_tx_sidl)
            {
                comparison_results[i] = 0;
            }
            else
            {
                comparison_results[i] = 1;
            }

            continue;
        }
        // special case for EID8 and EID0 (since these bytes do not matter, we can omit them)
        if (i == 2 || i == 3)
        {
            comparison_results[i] = 0;
            continue;
        }
        // special case for DLC that differs between TX and RX
        if (i == 4)
        {
            // 1 << 6 (bit 6, RTR)
            // 0xF = 1111 (bits 3-0)
            uint8_t masked_rx_dlc = rx_frame_bytes[i] & ((1 << 6) | (0xF << 0));
            uint8_t masked_tx_dlc = data_payload[i] & ((1 << 6) | (0xF << 0));

            if (masked_tx_dlc == masked_rx_dlc)
            {
                comparison_results[i] = 0;
            }
            else
            {
                comparison_results[i] = 1;
            }

            continue;
        }
        if (rx_frame_bytes[i] == data_payload[i])
        {
            comparison_results[i] = 0;
        }
        else
        {
            comparison_results[i] = 1;
        }
    }

     // ----- Loopback -> Normal Operation mode -----

// request Normal mode (bits 7-5: 000)

// 0b1110 0000 = 0xE0 <- mask, bits 7-5 are requested to change
// 0b0000 0000 = 0x00 <- data byte, bits 7-5 are requested to changed to 000
mcp2515_bit_modify(CANCTRL1, 0xE0, 0x00);

// waiting loop for mode to change
// mask: 0b1110 0000 = 0xE0 - bits 7-5 are relevant only

mcp2515_poll_bit_timeout(CANSTAT1, 0xE0, 0x00, 10U, &isSuccess);

// mode has not been set when the timeout hit
// indefinite red led error blink
if (isSuccess != 1)
{
    while (1)
    {
        led_on(&ledHandle);

        SysTick_Delay_ms(300);

        led_off(&ledHandle);

        SysTick_Delay_ms(300);
    }
}

// if the mode has been set to Normal Operation mode

// the variables below has already been declared and populated previously in the code while Loopback test harness
/* // test id
 // since it is 11-bit long, uint16_t is the type that can hold the value without truncation
 // bits 15-12 are zeroed out
 id = 0x1F2;

 SIDH = id >> 3;

 // EXIDE and EID bits left at 0 for standard frames
 SIDL = (id & 0x7) << 5;

 bytes_number = 5;

 // bit 7: unimplemented, 0
 // bit 6: RTR, in this case for a data frame, 0
 // bits 5-4: unimplemented, 0
 // bits 3-0: number of data bytes to be transmitted (0-8 bytes)
 // 5 bytes = 0b0101 => in 8 bit representation: 0000 0101 (RTR explicitly cleared with 0)
 DLC = bytes_number;

 // SIDH, SIDL, EID8 (zeroed out, don't care), EID0 (zeroed out, don't care), DLC, 5 data bytes
 uint8_t data_payload[10] = {SIDH, SIDL, 0x00, 0x00, DLC, 0x3F, 0xF1, 0xB4, 0x11, 0xAA};
 uint8_t load_tx_buffer_length = 10;

mcp2515_load_tx_buffer(MCP_Load_TXB0SIDH, data_payload, load_tx_buffer_length);

mcp2515_rts(&location, 1U);

return 0;
}
*/

int main(void)
{
    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    spi_init(SPI_BR_8);

    // error led initialization
    LED_HandleTypeDef ledHandle = {
        .pin = 0,
        .rcc_bit = 0,
        .moder_reg = &(GPIOA->MODER),
        .odr_reg = &(GPIOA->ODR),
        .rcc_clk_reg = &(RCC->AHB1ENR)};

    led_init(&ledHandle);
    // error led initialization

    uint8_t isSuccess;

    mcp2515_reset();

    uint8_t rx_byte = 0;

    mcp2515_read(CANSTAT1, &rx_byte, 1U);

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

    // mode has not been set when the timeout hit
    // indefinite red led error blink
    if (isSuccess != 1)
    {
        while (1)
        {
            led_on(&ledHandle);

            SysTick_Delay_ms(300);

            led_off(&ledHandle);

            SysTick_Delay_ms(300);
        }
    }

    // // poll the RX0IF in READ_STATUS
    // // RX0IF - receive buffer 0 Full Interrupt Flag bit
    // // when RX0IF is 1 - interrupt is pending (must be cleared by MCU to reset the interrupt condition)
    // uint8_t status_val = 0;

    // // TODO
    // // an unbounded poll - a deliberate, correct, test-harness wait for a human-triggered external event, with a clear exit condition (cansend)
    // // should be refactored into the timeout-and-defined-behavior version for a real deployment (or the Week 21 integration project)
    // do
    // {
    //     mcp2515_read_status(&status_val, 1U);
    // } while (!(status_val & (1 << 0)));

    /*
    // now this EFLG polling loop is unbounded
    // a real deployment would want this non-blocking (checked periodically rather than blocking main())
    do
    {
        // read the EFLG register (0x2D) awaiting for the RX0VR
        mcp2515_read(EFLG, &status_val, 1U);
    } while (!(status_val & (1 << 6)));
    // after the second frame arrived, causing OVR, the bit 6 RXOVR is set, so the loop exits

    // once RX buffer has the data frame, retrieve it
    uint8_t rx_frame_bytes[9];

    mcp2515_read_rx_buffer(MCP_Read_RXB0SIDH, rx_frame_bytes, 9U);
    // the READ RX BUFFER instruction automatically clears the associated receive flag, RXnIF (CANINTF), when CS is raised at the end of the command
    // so the RX0IF flag will be cleared automatically, so the hardware condition to reset the interrupt condition is satisfied
    */

    uint8_t can_intf_val;

    // a data frame:
    // SIDH, SIDL, EID8 (zeroed out, don't care), EID0 (zeroed out, don't care), DLC, up to 8 data bytes
    // 1 + 1 + 1 + 1 + 1 + 8 = 5 + 8 = 13
    uint8_t can_int_rx0_header[5];
    uint8_t can_int_rx0_payload[8];
    uint8_t can_int_rx0_flag = 0;

    uint8_t can_int_rx1_header[5];
    uint8_t can_int_rx1_payload[8];
    uint8_t can_int_rx1_flag = 0;

    while (1)
    {

        if (can_int_flag)
        {
            do
            {
                mcp2515_read(CANINTF, &can_intf_val, 1U);

                if (can_intf_val)
                {
                    mcp2515_canintf_handler(can_intf_val, can_int_rx0_header, can_int_rx0_payload, &can_int_rx0_flag, can_int_rx1_header, can_int_rx1_payload, &can_int_rx1_flag);
                }
            } while (can_intf_val != 0);

            can_int_flag = 0;
        }

        // if rx0_flag is set, the rx0_buffer has a new frame
        if (can_int_rx0_flag)
        {
            // for now, inspect the frame in the CGB
            // UART incorporation is the Integration Project scope

            can_int_rx0_flag = 0;
        }
        // if rx1_flag is set, the rx1_buffer has a new frame
        if (can_int_rx1_flag)
        {
            // for now, inspect the frame in the CGB
            // UART incorporation is the Integration Project scope

            can_int_rx1_flag = 0;
        }
    }

    return 0;
}