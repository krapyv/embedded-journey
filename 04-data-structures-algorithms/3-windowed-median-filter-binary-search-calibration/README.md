# Sensor pipeline: median filter + binary search altitude LUT

The project handles a windowed median filter feeding a binary-search calibration lookup table for temperature and pressure values measured by BMP280. Insertion Sort maintains the median filter's sorted window; Binary Search locates the calibration LUT entry for a given raw reading - both bounded, deterministic, and non-recursive. It sits on top of the existing BMP280 and UART drivers.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│    ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│    └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── devices/bmp280      
│    ├── bmp280.h        # Device handle, state machine enums, register map, and public API signatures
│    └── bmp280.c     # BMP280 peripheral driver: state machine functions, calculation and compensation functions
│
├── periph/     # Portable peripheral drivers
│    ├── i2c.c      # Register-level I2C peripheral driver: EV/ER ISR logic, transaction state machine, and SWRST recovery
│    ├── i2c.h      # I2C control and state enums, handler struct declaration, extern variable declaration, and function headers
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    ├── uart.h      # USART2 bit definitions, control macros, and function declarations
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, BRR, clock and register configuration and function headers
│
├── main.c      # Application entry point: BMP280 state machine, LUT, median filtering and binary search
├── app_config.h    # The header file for UART peripheral mode and window struct
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
└── lut_generator.py    # Python/scipy host-side script to generate a fixed-size lookup table representing pressure-altitude relation
```

## Breadboard circuit

- VCC is connected to the power rail (red jumper wire).
- GND is connected to the Ground rail (black jumper wire).
- CSB is connected to the power rail  (orange jumper wire). If the CSB is connected to the VDDIO (VCC), the I2C interface is active. Otherwise, if the CSB is connected to the GND (0V), the SPI interface is active. 
- SDO is connected to the GND rail (brown wire). The driver uses by default address 1110110 (0x76). 
* The driver uses by default SDA1 and SCL1:
- SDA of the BMP280 is connected to the PB7 of the Blackpill (green wire).
- SCL of the BMP280 is connected to the PB6 of the Blackpill (yellow wire).

* I2C is open-drain. The lines need pull-ups to VCC to define the idle high state. Without pull-ups configured either externally or internally, the lines float and the bus never reaches a defined idle state.
- In the breadboard circuit there were used 4.7k Ohms pull-up resistors on both SDA and SCL connected to power rails (rise time 217ns from 1000ns available).

* The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

## Architectural Decisions

1. Batching window instead of sliding window:
Chose batching (collect 7 fresh samples, sort once, emit one median) over sliding (resort on every new sample) - the physical quantities being sampled (room temp/pressure) don't move fast enough for continuous resorting to buy anything real.
Computed the real worst-case batch cost: 7 triggers * 6.4ms + I2C bus overhead (derived from 100kHz clock period * ~39 bits per transaction = approximately 0.5ms * 7) = 48.3ms total.

2. Compensate-then-filter, not filter-then-compensate:

Original assumption was to median-filter raw `adc_T`/`adc_P` first and run Bosch's compensation formula once on the resulting median (to avoid running the compensation math 7 times). Chased this down properly: median-of-transform equals transform-of-median only if the transform is strictly monotonic. Verified temperature's compensation formula actually is monotonic across the sensor's real -40°C to +85°C range by finding the quadratic's vertex (`-C1/C2`) and showing the large linear coefficient (`dig_T2`-driven) keeps it far outside the physical domain, dominating the tiny quadratic term (`dig_T3`-driven). But then found the real blocker: pressure's compensation formula needs its own `t_fine` - a value computed fresh per-sample from that instant's own `adc_T` - so `t_fine` must be computed for all 7 samples regardless, which means the "expensive" compensation math (the actual `var1`/`var2` work building `t_fine`) can never be defered to "once, after filtering" - only one trivial final shift-and-scale line was ever avoidable, and that saving didn't survive once `t_fine`'s mandatory per-sample cost was accounted for.
Separately confirmed a second reason raw values couldn't sit in the windows at all: filtering `adc_T` and `adc_P` independently could return medians from two different original samples, destroying the physical pairing pressure's formula depends on.
Final pipeline: each of the 7 triggers runs full temperature-then-pressure compensation immediately, and only the resulting physically-meaningful T and P values populate the two windows.

3. The lookup table made `const`:
Every value is known and fixed the moment the source file is written, so declaring the table as `const` tells the compiler to place it in `.rodata`, and the existing linker script already routes `.rodata` into the FLASH region - it becomes part of the `.bin`.

4. Table sizing:
First reached for "32 points" as a round number. Computed the true barometric midpoint over a 10m indoor test range against the perfectly linear midpoint between endpoints - deviation came out to 0.0001 hPa, far below the sensor's own +-0.16 hPa noise floor. Mathematically, 2 points already suffice for this range; settled on 4 anyway as a deliberate, stated choice to actually exercise the LUT/binary-search topic, not an unnoticed inefficiency.

4.1. Widening the table to 7:
On real hardware, the binary search returned "out of range" for a live pressure reading (1014.68 hPa) that should have bracketed cleanly.
First decision was to "enhance the const array" - widen its range.
Caught that this treats it as a table-sizing problem when it is not one: the table's 0m entry is a fixed constant (`scipy.constants.atm`, standard atmosphere, 1013.25 hPa), but real atmospheric pressure at a fixed location drifts with weather day to day, independent of altitude entirely - a static table anchored to one hardcoded `P0` has no way of being correct on more than a lucky day.
Confirmed this is exactly why real barometric altimeters require a live, current local sea-level reference pressure (QNH) as an input, not a baked-in-constant.

Decided not to rebuild the pipeline around a live-P0 recalculation, even though that's the technically correct fix - reasoned explicitly that doing so would replace the sort/LUT/binary-search mechanism this project exists to teach with a different algorithm entirely, which is not the point of the exercise. 
Instead: widened the table's altitude range to include realistic negative values (since local pressure now regularly exceeds the fixed 0m reference), fixed a follow-on bug this surfaced (the LUT's altitude field was still unsigned, so a negative bracket would have wrapped to a huge positive number), and documented the fixed-P0 assumption explicitly as a stated scope boundary - a TODO at the point of use.

5. The temperature and pressure window buffers without `volatile`.
There are no ISRs, no interrupts at all, the BMP280 works only in the thread mode I2C. Because of that `volatile` is not needed on the windows.

6. Insertion sort instead of other "textbook fast" algorithms:
Insertion sort on a tiny fixed-size window is worse in big-O terms that quicksort - recursive, unbounded stack depth, unpredictable worst case. Since Insertion sort is bounded and predictable it is a much better option.

7. The Python script that generates `{pressure, altitude}` pairs offline:
The STM32F411 does have a hardware FPU that accelerates basic operations - add, multiply, divide, square root. `exp()` does not have that support - it is a software library routine, typically a polynomial or series approximation running for a variable number of iterations depending on the input. That's exactly the kind of unbounded cost.
So to avoid using `exp()` on the STM32F411, the Python script was created to generate the table and copy it once to the .c file.

8. The lookup table pressure values as Q24.8 fixed-point:
The script printed plain float Pascals, but `BMP280_Pressure_Compensate()` actually returns Q24.8 fixed-point (pressure * 256, packed into a `uint32_t`. 
Chose to convert the table itself into Q24.8 at generation time (cheap, done once, ever) rather than converting the sensor's median value every batch - checked that the float->Q24.8 truncation this introduces (~0.2 raw units, = approximately 0.0000078 hPa) is negligible against the noise floor (+-0.16 hPa).

## Known Limitations

1. Insertion sort's `int32_t*`-typed function is called on the `uint32_t*` pressure window via pointer reinterpretation - safe today because real pressure values (~26M) sit far below the `int32_t` sign-bit threshold (~2.1B), so the bit pattern reinterprets identically either way. 
This is technically a strict-aliasing violation under the C standard: if the compiler ever sees the same memory accessed through both an `int32_t*` and `uint32_t*` view within a context it can optimize across, it is permitted to assume the two accesses don't alias and reorder or eliminate one of them, silently breaking the sort.
Not fixed - would require either a duplicate `uint32_t` sort function or a `memcpy`-based type-punning workaround - because no such dual-typed access pattern currently exists in this codebase for the optimizer to exploit.
