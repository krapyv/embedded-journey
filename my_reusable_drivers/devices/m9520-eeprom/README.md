# M95320 EEPROM SPI Driver

A driver built on top of the SPI blocking driver, abstracting raw byte transfers into write enablement, WIP poll, read and write cycles.
The `eeprom_write_byte`() incorporates the WEL bit enabling via `eeprom_write_enable`() as well as `eeprom_wip_poll`() after the transfer to wait for the internal memory write cycle to complete. The `eeprom_read`() handles burst reads up to 4096 bytes length (full Memory Array) in one go.
`eeprom_wip_poll`() polls the WIP bit (bit 0) in the STATUS register until the internal write cycle completes - the M95320 requires up to 5ms after a WRITE instruction before the memory array is ready.
This documents the application-layer usage with SPI driver.

## Project Structure

```text
reusable_drivers/
├── devices/m95320-eeprom
│    ├── m95320.h        # Instruction opcode macros and public API signatures
│    └── m95320.c     # M95320 peripheral driver: transaction and poll functions
└── periph/     # Portable peripheral drivers
    ├── spi.c      # Register-level SPI peripheral driver: initialization and transfer functions
    └── spi.h      # SPI_Channel_t enum for future multi-channel expansion and function headers
```

## Public API

### eeprom_read
Issues the EEPROM read transaction, waits for its finish and returns.

eeprom_read() must be called from the application level files, e.g. main.c.

Parameters:
- address      Pointer to address two bytes transmit buffer. Must not be NULL. Every read transaction has address bytes.
- rx           Pointer to receive buffer.
- len          The length of bytes-to-read. Length is allowed to be more than 0 and less than 4096 bytes - 4096 bytes is the size of the Memory Array of M95320.

Returns only after the SPI transaction is finished.

### eeprom_write_byte
Issues the EEPROM write transaction, waits for its finish and returns.

eeprom_write_byte() must be called from the application level files, e.g. main.c.

Parameters:    
- address      Pointer to address two bytes transmit buffer. Must not be NULL. Every write transaction has address bytes.
- data_payload       Pointer to the buffer of bytes-to-be-written.
- len          The length of bytes-to-be-written. Length is allowed to be more than 0 and less than 32 bytes - 32 bytes is the page size of M95320.

Returns only after the SPI transaction is finished.

Every write transaction requires Write Enable Latch to be 1, so the function internally calls `eeprom_write_enable`() to handle it.

After the SPI transaction has finished, the `eeprom_write_byte`() internally calls `eeprom_wip_poll`() to wait for the internal write cycle to finish because no new write or read transaction is allowed during an ongoing internal write cycle.

`eeprom_write_enable`() and `eeprom_wip_poll`() are called internally - the user does not need to call them directly.

## Low-lever Architectural Decisions
1. Length guards in `eeprom_read` and `eeprom_write_byte` functions. Since the Read operation can be executed on the whole memory array (4096 bytes) and the Write operation can be used for the full page (32 bytes) in one burst, these numbers became the highest threshholds for the Read/Write operations. Len == 0 has become out of scope as well since Read and Write with length of 0 does not fulfill their purpose.
2. The `eeprom_wip_poll` function is called in `eeprom_write_byte` after the `spi_transfer`() returns. Even if the SPI transaction has finished on the lines, the internal write inside the EEPROM takes up to 5 milliseconds of time, so to prevent unwanted situations when the read or new write is issued within these 5 ms, we need to return from the `eeprom_write_byte` only after the WIP bit in Status register has become 0.

## Known Limitations
1. The length checking guards use magic numbers (4096, 0, 32). Even though they do not hurt the program in any way and are doing their work as expected, we need to use #define macros instead of magic numbers.
2. The current version of the driver does not implement any function for Write Status Register (WRSR) or bit checking via Read Status Register (RDSR) except for WIP bit. The driver scope is limited to basic read/write cycles - WIP polling is the only STATUS register operation needed for that scope.
