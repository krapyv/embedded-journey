# Bootloader

The project handles receiving a binary over UART and writing it to Flash Sector 1. Before jumping, the bootloader validates the application's stack pointer by checking that the first word at the application start address falls within the SRAM boundary.  Then it jumps to the application. 
The application blinks an LED to confirm successful boot. The bootloader is built on top of the UART and SysTick reusable drivers.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│     ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│     └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── periph/     # Portable peripheral drivers
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    ├── uart.h      # USART2 bit definitions, control macros, and function
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, BRR, clock and register configuration and function headers
│
├── main.c      # Bootloader entry point: flash operations, jump sequence, HardFault_Handler, UART Chunk-Receive protocol
├── app_config.h    # The header file for UART peripheral choosing TX_RX_POLLING mode
├── flash_config.h # Project flash states and structs 
├── file.bin    # The .bin file of the application transferred by UART
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
└── script.py    # Python/python-serial host-side uploader script
```

## Breadboard circuit

The push tactile button:
* **The button itself**: Straddles the center ravine.
* **STM32F411 Pin PB13**: Connected to a row sharing one terminal of a tactile button.
* **4.7 kOhms pull-up resistor:** the first terminal is connected to the row of the PB13 wire and the first button terminal. The second terminal is connected to the power rail (3.3V).
* **GND wire:** Connected to the third terminal of the button (on the another side of the ravine) that is isolated from the first terminal. Connected to the GND rail. 

Button released (pin reads 1) -> jump to application.
Button held at reset (pin reads 0) -> stay in bootloader.

The LED:
* **STM32F411 Pin PA5**: Connected to a row sharing one terminal of a 220 Ohms Resistor.
* **Resistor (Terminal 2)**: Connected to the Anode (longer leg) of the LED.
* **LED Cathode (shorter leg)**: Connected to a common ground rail tied directly to the GND pin of the Blackpill.

The USART-to-USB adapter:
* **GND**: Connected to the GND power rail (black wire).
* **TXD:** Connected to the breadboard row of PA3 (RX2) (blue wire).
* **RXD:** Connected to the breadboard row of PA2 (TX2) (yellow wire).
* **VCCIO is not connected.**

## Architectural decisions
1. UART Chunk-Receive protocol design and implementation:

*Packet layout:* 
Start_byte(1) -> payload_len(1) -> payload[128] -> checksum (16-bit additive, big-endian: high byte, then low byte) -> end_byte (1). Total: 133 bytes per chunk.

Chose additive checksum over CRC deliberately - simpler arithmetic, acceptable blind spot (insensitive to byte reordering and certain paired bit-flips) for a short physical UART link with mostly single-bit/burst noise. Documemted explicitly that this is a known limitation, not an oversight - CRC upgrade is legitimate future scope.

Payload_len kept as a meaningful field (not redundant with the fixed 128-byte wire size) - it tells the bootloader how many of the 128 bytes are real image data versus 0xFF padding on the final chunk. Padding with 0xFF up to a 4-byte multiple is provably safe: erased flash is all-1s, and writing 1-bits onto already-erased flash changes nothing electrically - confirmed against the manual's own note that changing bits from 1 -> 0 requires an erase, but writing 1 onto 1 requires nothing.

End_byte kept deliberately as a resync sanity check, and for future-proofing if payload size ever becomes dynamic instead of fixed.

*Sentinel packet:*
Payload_len = 0 signals end-of-transfer. Confirmed against my own state machine: UART_PAYLOAD_LEN case checks payload_len == 0, sets is_last = 1 immediately, never proceeds to read payload/checksum/end_byte for that packet - so the sentinel really is only 2 bytes on the wire (Start_byte + Payload_len = 0), not a full 133-byte packet with dummy filter.

*Reception mechanism:*
Polling loop, not interrupt-driven. Justified from the actual timing: the protocol is strictly half-duplex request/response (host sends chunk, waits, bootloader ACKs/NACKs, host sends next) - the bootloader has nothing else to do while waiting, so a blocking poll loop is simpler and just as correct as interrupts here.

*Inter-byte timeout derivation:*
Baud rate 115200, 8-N-1 framing = 10 bit-frames per byte = 86.8 µs per byte on the wire. Wanted roughly 5x margin over that (not too tight, not too loose) = ~434 µs.
But SysTick resolution is 1ms per tick, and a 1-tick countdown has a real hazard: if the start measurement lands one cycle before the tick boundary, the "1 tick" timeout can fire after almost no real time has elapsed at all. Minimum safe value is 2 ticks (2ms), guaranteeing at least 1 full tick of real margin regardless of phase alignment. 2ms against a required 434µs margin is actually a ~23x margin, not 5x - but the binding constraint became tick resolution, not the original margin target. No cost to this looseness in the happy path - timeout value only affects failure-detection latency, never throughput, since every real byte arrives well within 86.8µs and reloads the countdown long before it could expire.

Chose inter-byte timeout (resets on every byte arrival) over a whole-chunk fixed deadline - a whole-chunk deadline sized around the flash-timing budget would false-fail a slow-but-steady host (e.g. 128 bytes at 50ms gaps = 6.4s total, failing every chunk even on a healthy link). Inter-byte timeout only fires on a genuine stall, regardless of how slowly the rest of the chunk trickles in.

2. Protocol and why raw streaming is unsafe:
During flash erase or programming, instruction fetches from flash stall. Sector erase time - 16 KB sector max 500 ms. During that 500 ms window, the bootloader's own UART ISR code (sitting in flash) cannot be fetched. Incoming UART bytes arrive with nothing pulling them from DR. RXNE stays set. Next byte arrives before DR is read - RXNE still set when new data arrives in shift register. ORE (Overrun Error) asserts. The incoming byte is lost. Protocol desynchnorizes.

Fixed-chunk ACK/NACK protocol solves this perfectly: host sends one chunk, waits for ACK before the next. Host timeout must exceed max erase time + max program time with margin - a too-short host timeout causes retransmission into a deaf bootloader, ORE, desync.

3. Bootloader design:

*Partition finalized:*
Bootloader = Sector 0 + 1 (32 KB), application = sector 2 + starting at 0x08008000.

*Boot trigger:*
GPIO pin read at reset. RTC_BKPxR (backup registers, requiring DBP unlock in PWR_CR) deferred - unnecessary complexity for v1.

*Flash unlock sequence:*
* Write KEY1 (0x45670123) and then KEY2 (0xCDEF89AB) to FLASH_KEYR.
* Check that LOCK bit (bit 31) of FLASH_CR reads 0 to confirm unlock. 
Any wrong key or wrong order re-locks and sets an error flag.

*Erase sequence:*
* Poll BSY (bit 16 FLASH_SR).
* Set SER (bit 1 FLASH_CR).
* Write SNB`[3:0]` (bits 6:3 FLASH_CR) with target sector number.
* Set STRT (bit 16 FLASH_CR).
* Poll BSY until clear.
* Check error flags.

*Program sequence:*
* Poll BSY.
* Set PG (bit 0 FLASH_CR).
* Set PSIZE = 10 (bit 9:8 FLASH_CR) = 32-bit parallelism - legal at VDD = 3.3V (minimum 2.7V).
* Write 32-bit word to target flash address.
* Poll BSY.
* Check error flags.
* Repeat for each word in chunk.
* After last word confirmed - clear PG.
* PG is held across the entire word-write loop - no window where a write happens with PG deasserted.

*5 unconditional bits of FLASH_SR error flags:*
WRPERR (bit 4), PGAERR (bit 5), PGPERR (bit 6), PGSERR (bit 7), RDERR (bit 8, included defensively, can never fire in currect configuration). All set regardless of interrupt enable state.

OPERR (bit 1) and EOP (bit 0) are gated by ERRIE and EOPIE respectively - excluded from the error-check list.

*Application validation before jump:*
SP range (0x20000000 - 0x2001FFFF) + alignment (bits 2:0 = 000) only. No image CRC.

*Jump sequence:* 
Validate SP -> MSR MSP word0 -> write SCB_VTOR to 0x08008000 ->branch to word1 (Reset_Handler address from vector table[1]).

4. GPIO Boot-Trigger:

*Pin/resistor:*
PB13, chosen because it's unused by any other driver in the project. External pull-up used exclusively (consistent with never using internal PUPDR in any project) - originally planned 5kΩ, switched to 4.7kΩ once that's what was actually on hand (as I found out, 5kΩ is not even a standard resistor value). 4.7kΩ is the closer standard part anyway, and exact resistance does not actually matter for a clean digital logic read.

*Polarity decision:*
Button open -> pin reads 1 (via external pull-up to VDD) -> jump to application. Button held -> pin reads 0 (shorted to GND) -> stay in bootloader.
Chose this deliberately as the safer default: a dead battery, unpressed button, or normal power-up never accidentally strands the device in bootloader mode - deliberate action is required to enter the exceptional path, not the normal one.

*Debounce:*
Worked out the actual settle-time math rather than assuming a delay was needed. 
At 16MHz HSI, 1 cycle = 62.5ns. Worst case bounce settle time ~9ms = 144 000 cycles. But this number assumes the button press and MCU reset happen at the exact same instant, which they never do in practice - the user is holding the button down well before or during reset, contacts settle within the first few ms, and the user continues holding for hundreds of ms to seconds. By the time Reset_Handler finishes zeroing .bss, copying .data, and main() reaches the actual GPIO read (2-3 instructions), the button has already been in a steady settle state for far longer than the bounce window. No debounce delay needed - confirmed by tracing actual timescales rather than assuming a guard was required.

## Known Limitations

1. The bootloader does not have self-update functionality - bootloader rewriting its own flash sector while running from it is out of scope for v1. The bootloader is flash-once-via-SWD.

2. UART Chunk-Receive protocol uses checksum over CRC.

3. Lack of the backup registers while boot trigger:
Boot trigger relies solely on GPIO pin state at reset, not RTC backup registers. This means the bo
otloader cannot be triggered programmatically by the application itself (e.g., 'reboot into bootloader mode' command) - only a physical button press at power-on works.

4. The application can occupy only Sector 2 (16 KB) due to lack of support for expansion on the next sectors:
16KB is a hard ceiling on application binary size - sufficient for a single-driver demo application, but likely too small for a multi-driver integration project without extending into Sector 3.

5. The UART chunk-receive protocol implements XMODEM-like fixed 128-byte reception mechanism. The next step would be to implement the full XMODEM functionality.

6. The UART chunk-receive protocol (uart_chunk_receive_protocol()) does not check the first byte to be equal to 0xAA as well as the last byte to be equal to 0xBB. Without start/end byte validation, a desynchronized stream could be misinterpreted as valid packet data, corrupting the flash write.

## Next Steps

1. Make the bootloader a reusable driver: extract flash operations, jump sequence, and validation into a `bootloader.c`/`.h` pair with a configurable partition table, so future projects don't hardcode Sector 0+1/Sector 2 boundaries in `main.c`.
