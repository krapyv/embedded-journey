# Independent Watchdog Reusable Library

The project provides a highly reusable, hardware-agnostic way to configure and use the Independent Watchdog (IWDG) on STM32 microcontrollers. It ensures robust runtime system monitoring by employing a defensive synchronization loop with a timeout mechanism. This guarantees that even if the peripheral's clock domain fails, the CPU has a deterministic path to escape infinite stalls during initialization.
There are no hardcoded register addresses or bloated external vendor libraries. This implementation relies entirely on standard C data types and direct register manipulation.

## 📂 Project Structure

```text
├── iwdg.c       # Initialization and kicking functions implementations
└── iwdg.h       # Struct layouts, enumerations, and driver function prototypes   
```

## 🧠 Core Low-Level Concepts
### Memory Map Base Addresses

* **Dynamic Peripheral Mapping:** The absolute base address (`IWDG_BASE_REG`) is passed at runtime via an instance of the configuration handle (`IWDG_HandleTypeDef`).
* **Pointer Arithmetic Offset:** Sub-registers (`IWDG_PR`, `IWDG_RLR`, `IWDG_SR`) are indexed dynamically via types pointer offsets relative to the base address, avoiding hardcoded macros and enabling single-driver multi-instance support if needed on dual-watchdog platforms.

### Register Configuration & Bitwise Operations
This driver operates completely via raw pointer manipulation using an offset-based layout from the passed base register address:
1. Key Register (`IWDG_KR` / Offset +0):
    - `0xCCCC`: Armed an starts the watchdog counter.
    - `0x5555`: Disables write protection for the Prescaler and Reload registers.
    - `0xAAAA`: Refreshes ("kicks") the counter back to its reload value.

2. Prescaler Register (`IWDG_PR` / Offset +1): Configures the internal clock divider (4 to 256).

3. Reload Register (`IWDG_RLR` / Offset +2): Holds the countdown value (0 to 0xFFF).

4. Status Register (`IWDG_SR` / Offset + 3): Polled to verify when register updates are complete.

## 🛠️ Hardware Architecture Overview
## Defensive Synchronization Loop

To handle the asynchronous clock domains between the core CPU clock and the Low-Speed Internal (LSI) oscillator, the driver implements a masked timeout loop:

```c
    volatile uint32_t timeout_counter = 100000;
    while (((*(wdg->IWDG_BASE_REG + 3) & 0x03) != 0x00) && (timeout_counter > 0))
    {
        timeout_counter--;
    }
```

- **Masking (`0x3`)**: Isolates the `PVU` (Prescaler Value Update) and `RVU` (Reload Value Update) flags to guarantee forward-compatibility if upper bits of the status register change in later chip revisions.
- **Timeout escape**: Prevents a permanent CPU hand if the LSI silicon block experiences physical failure or fails to oscillate.

## 📐 Mathematical Modeling (Watchdog Window)

The Independent Watchdog is clocked by an internal, low-power RC oscillator (`LSI`) operating at approximately 32 kHz.

The timeout period ($T_{out}$) is governed by the following system equations:

$$
f_{cnt} = \frac{f_{LSI}}{\text{Prescaler}}
$$

$$
T_{out} = \frac{1}{f_{cnt}} \times \text{Reload Value}
$$

### This test configuration

$$
\begin{align*}
f_{LSI} &= 32{,}000\,\text{Hz} \\
\text{Prescaler} &= 64 \quad (\text{IWDG\_PRESCALER\_64}) \\
\text{Reload Value} &= 1{,}000
\end{align*}
$$

$$
f_{cnt} = \frac{32{,}000\,\text{Hz}}{64} = 500\,\text{Hz} \implies 2\,\text{ms per tick}
$$

$$
T_{out} = 2\,\text{ms} \times 1{,}000 = \mathbf{2{,}000\,\text{ms} \ (2.0\ \text{Seconds Window})}
$$

## 🔬 Hardware Verification & Validation Logs

This library was validated by running an asynchronous loop that kicks the watchdog for 4.0 seconds, intentionally enters a blocking infinite loop to simulate a system task lockup, and captures the resulting recovery event.

### Target Serial Terminal Output (via minicom @115200 baud):

```
--- MCU Boot Sequence ---                                                       

[ALERT] System recovered from a WATCHDOG RESET!                                 

[INFO] Arming the Independent Watchdog...                                       

[INFO] Watchdog Active. Running normal loop for 4 seconds.                      

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

System Status: OK (Kicking Dog)                                                 

                                                                                
!!! CRITICAL FAILURE: Software entry into infinite loop !!!                     

Stopping all watchdog kicks. Reset expected in 2.0s...                          
                                                                                

--- MCU Boot Sequence ---                                                       

[ALERT] System recovered from a WATCHDOG RESET!
```

## 🚀 Compilation and Integration
### Compiling as an Isolated Object File

Since this library does not contain a `main()` function, it is compiled directly into a reusable object file (`.o`) using the GNU ARM Embedded Toolchain (`arm-none-eabi-gcc`).

Run the following command to compile the source code without invoking the linker:
```bash
arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb -Wall -O2 iwdg.c -o iwdg.o
```
- `-c`: Compiled the source file into an object file (`iwdg.o`) but skips the linking phase.
- `-mcpu=cortex-m4 -mthumb`: Targets the specific ARM Cortext-M4 architecture layout of the STM32.

### How to Integrate This Library into Your Project

To use this library in an application, include the header file in your main application file and link the compiled object files together.

```c
#include "iwdg.h"

int main(void) {
    IWDG_HandleTypeDef watchdog;
    watchdog.IWDG_BASE_REG = (volatile uint32_t *)(0x40003000);
    watchdog.prescaler_val = IWDG_PRESCALER_64;
    watchdog.reload_val = 1000;
    
    IWDG_init(&watchdog);

    while(1) {
        IWDG_kick(&watchdog);
    }
}
```

### Final Linking Command

When building your final application image, link your application object files alongside this library's object file:

```bash
arm-none-eabi-gcc main.o iwdg.o -mcpu=cortex-m4 -mthumb -T linker_script.ld -o final_application.elf
```

## 🗺️  Next Steps & Learning Roadmap

This Watchdog driver is a core building block of **Phase 3 (ARM Bare Metal)** in my 24-month automotive embedded engineering plan.

To make this driver fully production-ready for automotive deployment, the next development steps for this library are:
1. **Windowed Watchdog Support**.
2. **Low-Power Mode Handling**.
3. **MISRA C Compliance**.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
