# Multiple Producers-Single Consumer Task Communication

This project handles two independent interrupt sources - MCP2515 SPI ISR that reads the CAN frames sent by the second CAN node and periodic timer TIM2 ISR that represents a periodic broadcast/heartbeat pattern - feeding one queue at different NVIC priorities with the main() as a single consumer - a struct-based FIFO moving discrete messages between independent execution contexts.
The application is built on top of the UART and MCP2515 reusable drivers.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│     ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│     └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── devices/mcp2515      
│    ├── mcp2515.h        # SPI instructions, CAN register addresses, state enums, and public API signatures
│    └── mcp2515.c       # MCP2515 peripheral driver:  transaction, ISR and util functions
│
├── periph/     # Portable peripheral drivers
│    ├── spi.c       # Register-level SPI peripheral driver: initialization and transfer functions
│    ├── spi.h       # SPI_Channel_t enum for future multi-channel expansion and function headers
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    ├── uart.h      # USART2 bit definitions, control macros, and function declarations
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, BRR, clock and register configuration and function headers
│
├── main.c      # Queue implementation, two producers (MCP2515 ISR and TIM2 ISR) and a consumer- main()'s pop()
├── app_config.h    # The header file for UART peripheral mode, enums and queue structs, function headers
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
└── script.py    # Python/python-can host-side script to generate and send CAN frames over CANable to the board
```

## Breadboard circuit

The TXS0108E level shifter is sitting on the breadboard straddling the separation channel. The MCP2515 module sits off-board (male pins, not breadboard-mounted) as well as the CANable USB to CAN debugger-analyzer. 

Rail split: right rails = 3.3V, left rails = 5V. TXS0108E straddles the separation row — VA (3.3V) side faces right rails, VB (5V) side faces left rails.
Level shifter placed directly behind Black Pill to keep A-side hop short per capacitance budget.

TXS0108E side A faces 3.3V power rail, side B - 5V power rail.

MCP2515 VCC is connected to the 5V power rail (red wire).
MCP2515 GND is connected to the common ground rail (GND) (black wire).
MCP2515 SCK is connected to the TXS0108E B1 pin (yellow wire).
MCP2515 SI (MOSI) is connected to the TXS0108E B2 pin (green wire).

MCP2515 SO (MISO) is connected directly to STM32 PA6, bypassing the level shifter. STM32 input pins are 5V-tolerant on this line, and MISO is driven by the MCP2515 - the 3.3V STM32 receiver correctly interprets the 5V logic high.
The wire color - blue.

MCP2515 CS is connected to the TXS0108E B3 pin (white wire).
MCP2515 INT is connected to the TXS0108E B4 pin (gray wire).

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

* The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

## Architectural Decisions

1. `head` and `tail` struct indexes declared as `volatile`, meanwhile `*buffer` does not need it.

`head` and `tail` need volatile since each is read by a context that did not write it, and nothing else forces a fresh reload. 
`buffer` itself does not need volatile - every access to buffer[tail]/buffer[head] computes its address from an already-volatile index, creating a data dependency the compiler cannot hoist around; the index's volatile qualifier transitively forces a fresh read of whatever memory it points to.

The C standard only guarantees ordering among volatile accesses relative to each other, as observed by the abstract machine. It says nothing about whether a compiler may reorder a non-volatile store (buffer[head] = message) relative to a following volatile store (queue->head = ...), because from the compiler's point of view, the non-volatile store has no observable side effect it's obligated to sequence against anything except other volatile/atomic operations touching the same object.
The reason it is safe in practice relies on a combination of GCC's specific compiler rules and the data dependency involved in updating the index. 
1) According to the C standard, an assignment to a volatile variable is an observable side effect, and there is a sequence point immediately after it is evaluated. 
2) GCC treats volatile as a harsh barrier for optimizations. In GCC's internal optimization passes, a volatile store is treated as an operation with unknown side effects that can change the entire state of the machine (since volatile is fundamentally meant for hardware registers).
Because GCC treats a volatile write as a potential trigger for an external hardware action, it adopts a conservative policy that GCC wil not move non-volatile memory operations past a volatile memory operation. 
To remain a reliable tool for embedded systems, GCC explicitly guarantees that volatile acts as a compiler-ordering barrier for all surronding memory operations.

The Cortex-M4 Technical Reference Manual specifies that its hardware implementation features a single entry strict FIFO write buffer. The core mechanically cannot perform out-of-order write operations, even to entirely different SRAM  adddresses or cache lines (the Cortex-M4 does not have an internal L1 data cache; it speaks directly to the bus matrix).

Because of all of that, `__DMB()` is omittable on this chip implementation for the project.

2. The second producer has been changed from BMP280 to TIM2.

During the implementation, after the design session was fully closed, discovered by checking the BMP280 datasheet that it has no dedicated interrupt pin at all - every interaction is polled I2C, driven synchronously from `main()`'s own state-machine loop, never from an ISR context.

Rather than accept SPSC (the easier route, since the correct MPSC machinery had already been built and justified), decided to introduce a genuine second hardware interrupt source - a periodic timer (TIM2) - to keep the MPSC mechanism.

BMP280 was fully removed from this project's scope.

3. Struct field ordering: the largest-alignment member first (a union)

Derived that placing the largest-alignment member first avoids leading padding - verified with a 3-member hypothetical (1-byte flag + 2-byte field + 16-byte union): union-first ordering gives 20 bytes total; flag-first ordering gives 24 bytes.

So the total message struct: union (16 bytes) -> `producer_type` (1 byte) -> 3 bytes padding = 20 bytes.

4. Queue size depth as a power of two for cheap masked wraparound (`(index + 1) & (SIZE - 1)`) instead of modulo.

5. Chose the reserved-slot approach to avoid the ambiguity of `head == tail` that can mean both `empty` and `full`. With SIZE = 16 (satisfying the power-of-two masking requirement from decision 4), there are 15 usable slots and 1 sacrificed slot that makes it a trivial cost against a guaranteed race if I would have chosen an option to track count of used elements separately - breaks the moment two producers are involved since it is shared accross two writers.

## Known Limitations

1. Lack of portability.

Push/pop ordering relies on Cortex-M4's single-entry write buffer draining strictly in-order - an implementation detail of the ARM Cortex-M4, not an ARMv7-M architectural guarantee.
Porting this queue implementation to a e.g. Cortex-M7 core will break it. The Cortex-M7 has an out-of-order, dual-issue pipeline with a true L1 data cache and a write buffer that handles write-merging and optimizations. The hardware will reorder writes to different SRAM addresses if it thinks it is more efficient, completely violating the buffer's safety. 

On a Cortex-M4 it works because the single-entry write buffer has no capacity to hold a second entry to reorder against.

2. The Python script produces and sends a finite sequence of 2047 frames.

The script does not support an infinite sending, so to test it more than once, you need to run it again.

3. The script tests a happy path, sending frames slowly, one at a time, at human pace.

4. Queue functions implementation lives in the main.c file, not a stand-alone reusable driver.

## How to run

Clone the repository, navigate to the project directory, and execute the toolchain commands:

```bash
# Clean previous build artifacts and compile the firmware binary
make clean
make all

# Flash the binary to the microcontroller using ST-LINK
make flash

# Check if the CANable is seen by the Linux
ip link show type can

# Configure and activate a CAN interface with the name can0 
sudo ip link set can0 up type can bitrate 500000
```

Then run the script in the project directory:

```bash
python3 script.py can0

# or

# grant the file "executable" permissions
chmod +x script.py
./script.py can0
```

In new terminal window or tab:
```bash
# Check what name the UART-to-USB adapter has
ls /dev/ttyUSB* /dev/ttyACM* /dev/serial/by-id/* 2>/dev/null

# Open an UART serial communication session using minicom
minicom -D /dev/ttyUSB0 -b 115200
```

And watch the UART messages.
