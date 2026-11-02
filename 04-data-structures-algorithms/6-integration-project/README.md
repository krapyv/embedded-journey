# Integration Project - Multi-sensor Logger

A project that integrates a fast channel (encoder speed with EMA), a slow channel (BMP280), CAN frame logging via the MCP2515, and CRC-protected records streamed over UART.

```
TIM1 encoder -> speed -> PI -> TIM4 PWM -> motor, every 15 ms in the TIM2 ISR. The EMA sits inside or outside the loop, selected by a build-time macro FILTER_MODE (default: outside loop). Fast channel, computed every 15 ms, logged every 75 ms in the sensor record.                                      ┐
BMP280 -> I2C (slow channel, about 125 measurements per second, of which only about 13 are sent - every 5 TIM2 ticks) ┤-> queue -> CRC-16 record (CRC-16/X-25) -> UART
MCP2515 -> CAN RX                                                                                                     ┘
```
The project built on the reusable drivers listed in Project Structure.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│    ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│    └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── devices/bmp280      
│    ├── bmp280.h        # Device handle, state machine enums, register map, and public API signatures
│    └── bmp280.c     # BMP280 peripheral driver: state machine functions, calculation and compensation functions
├── devices/mcp2515      
│    ├── mcp2515.h        # SPI instructions, CAN register addresses, state enums, and public API signatures
│    └── mcp2515.c       # MCP2515 peripheral driver:  transaction, ISR and util functions
├── periph/i2c     # Portable peripheral drivers
│    ├── i2c.c      # Register-level I2C peripheral driver: EV/ER ISR logic, transaction state machine, and SWRST recovery
│    └── i2c.h      # I2C control and state enums, handler struct declaration, extern variable declaration, and function headers
├── periph/spi
│    ├── spi.c   # Register-level SPI peripheral driver: initialization and transfer functions
│    └── spi.h   # SPI_Channel_t enum for future multi-channel expansion and function headers
├── periph/encoder
│    ├── encoder.c   # Quadrature encoder initialization and ReadDelta function
│    └── encoder.h   # Function headers declaration
├── periph/motor 
│    ├── motor.c   # Motor initialization and control functions
│    └── motor.h   # Motor timer clock, PWM frequency, PWM steps and function headers
├── periph/uart
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    └── uart.h      # USART2 bit definitions, control macros, and function declarations
├── periph/systick
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, BRR, clock and register configuration and function headers
├── utils/crc16 
│    ├── crc16.c      # CRC-16 table and calculate functions
│    └── crc16.h      # Function headers
├── utils/ema    
│    ├── ema.c   # EMA initialization and update functions
│    └── ema.h   # EMA control struct and function headers
├── utils/pid    
│    ├── pid.c   # PID control struct and function headers
│    └── pid.h   # PID initialization and update functions
├── utils/ring_buffer    
│    ├── ring_buffer.c   # Ring buffer initialization and control functions
│    └── ring_buffer.h   # Ring buffer control struct and function 
├── utils/queue    
│    ├── queue.c      # Queue initialization and control functions
│    └── queue.h      # Queue control struct and functions
│
├── main.c      # Application entry point: CAN EXTI ISR, TIM2 init and ISR, helper big-ending functions, logger frame functions, BMP280 state machine and CAN broadcast loop
├── app_config.h    # The header file for UART peripheral mode, structs and define values
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
└── logger_rx.py    # Python/pyserial host-side script-decoder for the multi-sensor logger stream
```

## Architecture Decisions

1. UART bandwidth budget

UART bandwidth budget sets the record rate. At 115200 baud (8-N-1) the link carries 11520 B/s. Two record types share it: CAN records (20 B each; 450 frames/s from the host - 9000 B/s) and sensor records (22 B). One sensor record per BMP280 measurement (~125/s, 2750 B/s) would bring the total to 11750 B/s, above capacity. Sending one sensor record every 5th control tick (75 ms, ~13.3/s = approx. 293 B/s) brings the total to approx. 9300 B/s, about 81% of the link. The encoder speed (EMA output) has no record of its own: it is computed every 15 ms and carried as a 4-byte field (offsets 15-18) in the sensor record, so it is logged every 75 ms.

2. Record Layouts

* CAN record:
Offset 0-1 Sync - 0xAA55
Offset 2 Type - 0x01
Offset 3-6 Timestamp - ms, SysTick
Offset 7-8 ID - 11 bits
Offset 9 DLC - 8 byte max
Offset 10-17 Data bytes - when <8, the unused bytes filled with 0x00, the decoder ignores them 
Offset 18-19 CRC - CRC-16/X-25, big-endian over Type to the end of payload

* Sensor record:
Offset 0-1 Sync - 0xAA55
Offset 2 Type - 0x02
Offset 3-6 Timestamp - ms, SysTick
Offset 7-10 Temp - int32, 0.01 °C
Offset 11-14 Pressure - uint32 Q24.8 Pa
Offset 15-18 Fast channel - int32, EMA accumulator = counts/tick * 2^5
Offset 19 Status - 0x01 - Fresh, 0x02 - Retrying, 0x03 - Dead
Offset 20-21 CRC - CRC-16/X-25, big-endian over Type to the end of payload

3. Byte order

The stream uses big-endian byte order because big-endian matches the bootloader protocol and the host decoder.
For that there were created two big-endian helpers: put16be and put32be.

4. Interrupt priorities

TIM2 has priority 1, SysTick - 2, I2C EV - 5, I2C ER - 6, CAN EXTI - 8, USART2 - 9.
The control loop is the most urgent and nothing can preempt it.
I2C sits above CAN so the 1-byte read's STOP deadline (80 µs) isn't blocked by the ~224 µs CAN ISR. Measured worst case 58 µs, a margin of 27.5%.
USART TX is the least urgent, because a late TXE only leaves a gap on the wire and never loses a byte.
CAN's BASEPRI = 1 window (~155 cycles) delays I2C EV entry but cannot occur inside the N=1 window, because CAN (8) cannot preempt EV (5). The delay falls while ADDR holds SCL low.

5. Event queue (MPSC)

There is one queue with two producers (CAN ISR, TIM2 tick) and one consumer (main).
Depth of 16: 14 CAN frames + 1 tick + 1 reserved slot, sized for a 3 ms main iteration at 100% bus load. A power of two, so indices use a mask.
The less urgent producer (CAN) raises BASEPRI to TIM2's level around its push. TIM2 needs no masking. 

16 slots absorb a burst; sustained CAN traffic above ~561 frames/s exceeds the UART budget and frames are dropped (counted per producer).

`pop()` does not need masking (see the MPSC README).

6. Shared data and ownership

Single-writer rule for every counter and index; each producer owns its own queue-full counter.
The motor logger struct `logger` (speed, PID output) is written by the TIM2 ISR and read by `main()`. `main()` reads the tick counter before and after copying it and retries if it changed (seqlock), with compiler barriers so the copy isn't reordered around the counter reads.
In both UART TX and RX ring buffers, the data access and the index index moves are separated by compiler barriers.

## Breadboard circuit

Two breadboards:
- the first contains the Black Pill and the Motor Driver TB6612FNG (A).
- the second one has the BMP280 and the level shifter TXS0108E (B).

All grounds are one nore: Black Pill GND, XL4015 OUT-, TB6612FNG GND, both breadboards' ground rails, MCP2515, CANable and the UART adapter.

**Pin map:**
| Pin    | Function |
| -------- | ------- |
| PA2 / PA3  | USART2 TX / RX    |
| PA4 / PA5 / PA6 / PA7 | SPI1 CS / SCK / MISO / MOSI (MCP2515)     |
| PA8 / PA9    | TIM1 CH1 / CH2 (encoder A / B)  |
| PB6 / PB7  | I2C1 SCL / SDA (BMP280)  |
| PB8  | TIM4 CH3 PWM (driver PWMA)  |
| PB12 / PB13 / PB14  | Driver AIN1 / AIN2 / STBY  |
| PB15  | EXTI15 (MCP2515 INT)  |


**Breadboard A:**

One power rail of the breadboard is connected to the 5V pin of the Blackpill (red wire).
The second power rail is connected to the 3.3V. 

1. Encoder: 

- VCC (5V) is connected to the Blackpill's 5V pin (red wire).
- GND is connected to the Blackpill's GND pin (black wire).
- A is connected to the PA8 (green wire).
- B is connected to the PA9 (blue wire).

Both PA8 and PA9 are 5 V-tolerant.

2. Power side:

- XL4015 OUT+ (red) is connected to the motor driver's VM.
- XL4015 OUT- (black) is connected to the driver's GND (one of three).
- The 470 µF capacitor across VM and GND, right at the driver. The stripe (negative) goes to GND. The long leg is positive.

3. Adapter 230 V AC 12 V DC

IMPORTANT: before connecting the driver, set the XL4015 output to 6.0 V and its current limit to 1 A.

The 12 V adapter connects through a DC socket with screw terminals. Check their polarity before wiring:
- Plug the adapter into the DC socket, then into the mains.
- Plug the multimeter on DC volts across the two screw terminals.
- It should read about +12 V with the red probe on the positive terminal. If it reads -12 V, the terminals are swapped. Mark the positive one with a bit of red tape.
- Unplug the adapter from the mains before touching any wiring.

Then wire the socket to the XL4015 input:
- Male pins into the socket's screw terminals.
- Red wire from the socket's `+` to the XL4015's `IN+`, and black from `-` to `IN-`.
- Check with the multimeter's continuity mode, adapter unplugged, that IN+ and IN- aren't shorted and neither are OUT+ and OUT-.

The XL4015 does:
- Steps the voltage down: it's a "step-down" or buck converter, so it takes 12 V in and gives a lower voltage out. You set the output to 6.0 V with the voltage pot, since the motor is rated for 6 V.
- Limits the current: the other pot sets a ceiling of about 1 A. If a bug or a stalled motor tries to draw more, the module holds the current at that value, so the driver doesn't overheat.
- Shows what is happening: the two displays give you the output voltage and the current, so you can see how much the motor draws at each duty.

It is powered from the adapter on its IN terminals, and it feeds the driver from OUT+ and OUT-. The Black Pill is powered separately over USB.

4. Logic side:
- Driver VCC is connected to the Black Pill's 3.3 V (power rail) (red wire).
- Driver GND is connected to the Black Pill's GND (black wire). All grounds are the same node: the XL4015 OUT-, the driver's GND, and the Black Pill GND.
- Driver STBY is connected to the Black Pill's PB14 (yellow wire).
- Driver's AIN1 is connected to the Black Pill's PB12 (white wire).
- Driver's AIN2 is connected to the Black Pill's PB13 (orange wire).
- Driver's PWMA is connected to the Black Pill's PB8 (purple wire).

5. Motor:
- Motor's red wire is connected to the driver's AO1.
- Motor's black wire is connected to the driver's AO2.

This sets which direction is forward: if `speed` comes out negative on the first forward spin, swap these two.

6. The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

**Breadboard B:**

One power rail of the breadboard is connected to the 3.3V of the first breadboard (orange wire). Second power rail is connected to the 5V rail of the first breadboard (orange wire).
Both ground rails of the breadboard B are connected to the ground rails of the breadboard A (brown jumper wires).

The TXS0108E level shifter is sitting on the second breadboard straddling the separation channel. The MCP2515 module sits off-board (male pins, not breadboard-mounted) as well as the CANable USB to CAN debugger-analyzer.

7. TXS0108E:

TXS0108E straddles the separation row — VA (3.3V) side faces right rails, VB (5V) side faces left rails.
Level shifter (the second breadboard generally) is placed close enough to the first breadboard to keep A-side hop short per capacitance budget.

TXS0108E side A faces 3.3V power rail, side B - 5V power rail.

MCP2515 VCC is connected to the 5V power rail (red wire).
MCP2515 GND is connected to the common ground rail (GND) (black wire).
MCP2515 SCK is connected to the TXS0108E B1 pin (yellow wire).
MCP2515 SI (MOSI) is connected to the TXS0108E B2 pin (green wire).

MCP2515 SO (MISO) is connected directly to STM32 PA6, bypassing the level shifter. STM32 input pins are 5V-tolerant on this line, and MISO is driven by the MCP2515 - the 3.3V STM32 receiver correctly interprets the 5V logic high.
The wire color - blue.

MCP2515 CS is connected to the TXS0108E B3 pin (white wire).
MCP2515 INT is connected to the TXS0108E B4 pin (gray wire).
MCP2515 J1 should be shorted (brown wire). J1 is 120Ω terminal resistance.

STM32 PA5 (SCK) is connected to the TXS0108E A1 (yellow wire).
STM32 PA7 (MOSI) is connected to the TXS0108E A2 (green wire).
STM32 PA4 (CS) is connected to the TXS0108E A3 (white wire).
STM32 PB15 (INT) is connected to the TXS0108E A4 (gray wire).

TXS0108E VA (3.3V side) connected to the 3.3V power rail (red wire).
TXS0108E VB (5V side) connected to the 5V power rail (red wire).
TXS0108E GND is connected to the common ground rail (GND) (black wire).
TXS0108E OE pin is connected to 3.3V power rail (orange wire).

The CANable GND is connected to the common ground rail (GND) (black wire).
The CANable CAN_H and CAN_L are connected to the MCP2515 CAN_H and CAN_L respectively. The CAN_H wire is yellow, the CAN_L wire is green.
The CANable is connected to the PC's USB port.
The CANable termination should also be enabled.

8. BMP280:

The BMP280 faces the right side of the power rails (3.3 V).

- VCC is connected to the power rail 3.3V (red jumper wire).
- GND is connected to the Ground rail (black jumper wire).
- CSB is connected to the power rail 3.3V  (orange jumper wire). If the CSB is connected to the VDDIO (VCC), the I2C interface is active. Otherwise, if the CSB is connected to the GND (0V), the SPI interface is active. 
- SDO is connected to the GND rail (brown wire). The driver uses by default address 1110110 (0x76). 
* The driver uses by default SDA1 and SCL1:
- SDA of the BMP280 is connected to the PB7 of the Blackpill (green wire).
- SCL of the BMP280 is connected to the PB6 of the Blackpill (yellow wire).

* I2C is open-drain. The lines need pull-ups to VCC 3.3V to define the idle high state. Without pull-ups configured either externally or internally, the lines float and the bus never reaches a defined idle state.
- In the breadboard circuit there were used 4.7k Ohms pull-up resistors on both SDA and SCL connected to power rails (rise time 217ns from 1000ns available).

## Known Limitations

1. 11-bit IDs only:
The CAN record's 2-byte ID field can't hold 29-bit extended IDs.

2. CAN frame-rate ceiling: 
Above about 561 CAN frames/s the budget is exceeded and frames are dropped (counted per producer).

3. Fast channel logged every 75 ms:
The speed is computed every 15 ms, but only one value in five is logged to prevent UART's budget excess.

4. Compiler barriers instead of data barriers:
For the Cortex-M4 compiler barriers are enough because there is a single-entry write buffer. For porting the driver to an M7, it would need `__DMB()`. 
On an M7 the write buffer can reorder stores to different addresses, so the queue, the ring buffers and the `logger` seqlock would need `__DMB()`. See the MPSC project for the full reasoning,
