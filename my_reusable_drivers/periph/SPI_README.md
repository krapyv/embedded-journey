# STM32F411 SPI Busy-wait Blocking Driver

A blocking SPI driver built around a single transfer function with dual TX buffers.
* TX is split into `settings_tx` (instruction and address bytes) and `data_tx` (payload) to avoid populating a contiguous buffer with dummy bytes when the receive length exceeds the instruction length - `data_tx` is set to NULL and the driver clocks dummy bytes automatically. 
* Both `data_tx` and `rx` buffers can be NULL to automatically handle dummy bytes clocking. 
* `rx_skip` discards the first N incoming bytes - the MISO echo of the instruction and address phase - before writing to the `rx`.
This documents the reusable driver layer only. Application-layer usage with EEPROM M95320WT is covered in a separate project README.

## Project Structure

```text
reusable_drivers/
├── core/   # STM32F411 register definitions
    └── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
└── periph/     # Portable peripheral drivers
    ├── spi.c      # Register-level SPI peripheral driver: initialization and transfer functions
    └── spi.h      # SPI_Channel_t enum for future multi-channel expansion and function headers
```

## Public API

### spi_transfer
Handles an SPI transaction, deselects the EEPROM and returns.

spi_transfer() must be called from the EEPROM driver level.

Parameters:
- settings_tx      Pointer to instruction + address bytes transmit buffer. Must not be NULL. Every transaction requires at least one instruction byte
- data_tx          Pointer to data transmit buffer. In read transfer can be NULL - the dummy bytes are used to generate clock pulses
- rx               Pointer to receive buffer. In write transfer can be NULL - the received bytes are read into a dummy variable
- total_len        The total length of the transfer (instruction + address bytes + `data_tx` if write or `rx` if read length)
- settings_tx_len  The length of `settings_tx` buffer
- rx_skip          The number of discarded bytes before populating `rx` buffer

Returns only after the bus is idle - CS deasserted and SPI1 BSY cleared.

## Low-level Architectural Decisions

1. TX buffer is split into `settings_tx` (instruction and address bytes) and `data_tx` (payload) to avoid populating a contiguous buffer with dummy bytes when the receive length exceeds the instruction length.
2. `rx_skip` discards the first incoming bytes before writing to the `rx` buffer to handle the MISO echo of the instruction and address phase.
3. ODR is set high before MODER is switched to output during `spi_init`(). Since the initial MODER mode is Input, the high ODR is not applied to the pin, but only queued. After the MODER changes to General Purpose Output, the pin immediately becomes high. By placing ODR driving the pin high after the MODER change, there would be a window when the pin in Output mode is driven low, that means the EEPROM is selected.
4. `spi_transfer()` uses `0xFF` as the dummy TX byte, keeping MOSI high during receive-only phases - consistent with SPI idle line state.

## Known Limitations

1. Hardcoded SPI channels and pins. The driver supports only SPI1 (NSS1 - PA4, SCK1 - PA5, MISO1 - PA6, MOSI1 - PA7) at the moment. 
2. Busy-wait blocking architecture. The CPU is 100% locked in transactions and busy wait loops, burning the CPU cycles.
In a multi-peripheral system, this prevents any other work from being done during SPI transactions. An interrupt-driven or DMA-based architecture would free the CPU during transfer.
