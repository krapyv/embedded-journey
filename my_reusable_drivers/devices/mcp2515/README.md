# MCP2515 CAN Interrupt-driven Driver

A driver that handles MCP2515 SPI instructions - read/write register, read/write buffer, request-to-send, and bit modify - to implement interrupt-driven CAN frame transmission and reception.
Since the STM32F411 does not have built-in CAN peripheral, this driver is built on top of the busy-wait SPI driver, allowing the board to communicate with other independent CAN nodes: STM32F411 -> SPI -> MCP2515 -> TJA1050 -> CAN bus.
This documents the reusable driver layer only. The application layer is covered in another README document.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│    └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── devices/mcp2515      
│    ├── mcp2515.h        # SPI instructions, CAN register addresses, state enums, and public API signatures
│    └── mcp2515.c       # MCP2515 peripheral driver:  transaction, ISR and util functions
└── periph/     # Portable peripheral drivers
    ├── spi.c   # Register-level SPI peripheral driver: initialization and transfer functions
    └── spi.h   # SPI_Channel_t enum for future multi-channel expansion and function headers
```

## Public API

### mcp2515_reset
Reinitializes the internal registers and set the Configuration mode.
The RESET function must be executed as part of the power-on initialization sequence.

After the SPI Reset, the Oscillator Start-up Timer keeps the device in Reset for 128 OSC1 clock cycles, and no SPI protocol operation should be attempted until the OST has expired. In function it is taken care of by 2 ms SysTick Delay.

Then the function returns.

### mcp2515_read
Issues an SPI RX transaction.

Parameters:
- addr          8-bit address
- rx_buffer     Pointer to receive buffer
- read_len      Number of bytes to receive (without the data bytes during the setting phase that would be discarded)

Returns after the SPI transaction is complete.

### mcp2515_read_rx_buffer
Provides a way to quickly address a receive buffer for reading. 
Reduces the SPI overhead by one byte, the address byte. Once the command byte is sent, the controller clocks out the data at the address location.

Parameters:
- location_mask     One from four options: RXB0SIDH, RXB0D0, RXB1SIDH, RXB1D0
- rx_buffer     Pointer to receive buffer
- read_len      Number of bytes to receive (without the data bytes during the setting phase that would be discarded)

### mcp2515_write
Issues an SPI TX transaction. 

Parameters:
- addr          8-bit address
- tx_buffer     Pointer to transmit buffer
- transmit_len  Number of bytes to write (without the data bytes during the setting phase)

Returns after the SPI transaction is complete.

### mcp2515_load_tx_buffer
Eliminates the eight-bit address required by a normal command. 
Allows quickly write to a transmit buffer that points to the "ID" or "data" address of any of the three transmit buffers.

Parameters:
- location_mask     One of six addresses: TXB0SIDH, TXB0D0, TXB1SIDH, TXB1D0, TXB2SIDH, TXB2D0
- data_payload      Pointer to transmit buffer
- write_len         Number of bytes to write (without the data bytes during the setting phase)

### mcp2515_rts
Used to initiate message transmission for one or more of the transmit buffers.

Parameters:
- locations         Pointer to an array of RTS locations: TXB0, TXB1, TXB2
- locations_len     Number of elements of the locations array

locations_len must not exceed 3 - the MCP2515 has three TX buffers. The function returns without action if guard is violated.

### mcp2515_read_status
Allows single instruction access to some of the often used status bits for message reception and transmission: TX2IF, TXREQ, TX1IF, TXREQ, TX0IF, TXREQ, RX1IF, RX0IF.

Parameters:
- rx        Pointer to the receive buffer
- read_len  Number of bytes to read

### mcp2515_rx_status
Used to quickly determine which filter matched the message and message type

Parameters:
- rx        Pointer to the receive buffer
- read_len  Number of bytes to read

### mcp2515_bit_modify
Provides a way for setting or clearing individual bits in specific status and control registers

Parameters:
- addr          Address of the register
- mask          8-bit mask - determines which bits in the register will be allowed to change: a 1 in the mask byte will allow a bit in the register to change, 0 - will not.
- data_byte     8-bit data payload - determines what value the modified bits in the register will be changed to: a 1 in the data byte will set the bit and 0 will clear the bit, provided that the mask for that bit is set to a 1.

### mcp2515_poll_bit_timeout
Polls a specified register bit until the expected value is reached or the timeout expires.

Parameters:
- addr              Address of the register
- mask              8-bit mask to isolate the bit/bits to poll
- expected_value    The expected masked register value that signals success
- timeout_ms        Time allowed to poll
- isSuccess          Pointer to the variable that sets to 0 or 1 depending on the result of the poll: 0 - not successful, 1 - successful

Returns after success or timeout. Caller must check \*isSuccess to determine the result - 0 means timeout expired before the expected value was observed, 1 means success.

## Low-level Architectural Decisions

1. The ISR EXTI15_10_IRQHandler is short: distinguishes what has caused the interrupt by looking in EXTI->PR register, clears the bit and sets the can_int_flag to signal that CAN INT line dropped to 0.
2. The handling of the individual interrupt bits of CANINTF happens in mcp2515_canintf_handle function called from the main() application layer.
3. In ERRIF interrupt handling only two error sources are acknowledged - RX0OVR and RX1OVR. To handler ERRIF, firstly clear all the set flags of EFLG using bit modify operation. After that, clear the ERRIF bit itself in CANINTF using the same bit modify operation. 
If the order will be reversed (the ERRIF bit clearing first, then EFLG handling) the ERRIF interrupt bit will be reset by hardware immediately. The ERRIF bit is tied to the EFLG register - if EFLG still has overflow flags set when ERRIF is cleared, the hardware immediately re-asserts ERRIF, causing an infinite interrupt loop.
4. To handle RXnIF interrupt bits, inside mcp2515_canintf_handler the buffer is read by mcp2515_read_rx_buffer function that internally uses READ_RX_BUFFER instruction. The command automatically clears RXnIF bit in CANINTF register, reducing the SPI overhead of explicitly calling mcp2515_bit_modify function to clear the flag.

## Known Limitations

1. Hardcoded INT pin PB15 and as a consequence EXTI Line 15. 
This prevents using multiple MCP2515 instances on the same board, each requiring a separate INT line.
The future refactored version should allow to use dynamically choosen pins for INT line. 
2. mcp2515_canintf_handler does not handle MERRF, WAKIF, TX2IF, TX1IF and TX0IE interrupt flags.
The handler should be expanded to cover all 8 CANINTF sources.
