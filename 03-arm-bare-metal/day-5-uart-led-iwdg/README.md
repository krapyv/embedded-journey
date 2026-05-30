# Independent Watchdog (IWDG) Integration & Fault Injection Testbench

This project implements a system-level integration testbench designed to validate the functionality, timing windows, and recovery cycles of the Independent Watchdog (`IWDG`) peripheral on STM32 microcontrollers. Operating completely bare-metal, the application brings together modular drivers for `USART2`, hardware `LED` indicators, and a non-blocking `SysTick` timebase to monitor the microcontroller's lifecycle during normal operations and simulated software anomalies.

This implementation explicitly samples hardware reset latches inside the Reset and Clock Control (`RCC`) block during the boot sequence to differentiate between a standard cold power-on cycle and a safety-critical watchdog recovery event.

## 📂 Project Structure

```text
├── main.c        # Application lifecycle manager, reset flag diagnostics, and fault loop
├── iwdg.c        # Independent Watchdog initialization and direct register pointer offset writes
├── iwdg.h        # Prescaler configuration enumerations and peripheral configuration structures
├── uart2.c       # USART2 configuration, clock gating, and ISR implementation
├── uart2.h       # Core hardware memory map, peripheral offsets, and NVIC flags
├── ring_buffer.c # Reusable thread-safe-conscious circular queue component
├── ring_buffer.h # Circular buffer status structures and pointer tracking
├── led.c         # Reusable bare-metal GPIO output driver  
├── led.h         # LED state handle abstractions and register maps
├── startup_stm32f411ceux.s       # Assembly startup file implementing vector table & Reset_Handle   
├── stm32f411.ld  # Linker script defining Flash/SRAM memory segments
└── Makefile      # GNU Make automation configuration to compile and flash the system
```

## 🧠 Core Low-Level Concepts
### Hardware Reset Flag Architecture & Diagnostics

When the microcontroller experiences a reset event, internal hardware latches inside the Control/Status Register (`RCC_CSR`) preserve the underlying root cause. This allows the application firmware to adapt its initialization sequence depending on the system's prior state.

1. **Watchdog Reset Flag Handling (`IWDG_RSTF`, Bit 29):** If the watchdog countdown timer reaches zero, it forces a hardware reset, asserting bit 29 (`IWDG_RSTF`) in `RCC_CSR`. The bootloader logic reads this bit to intercept recovered failures immediately upon system restart.

2. **Flag Clearing via Software (`RMVF`, Bit 24):** Reset flags inside `RCC_CSR` are sticky and remain set across subsequent soft resets. To ensure future faults are tracked accurately, the driver asserts the Remove Reset Flag bit (`RMVF`) by writing a 1 to bit 24, clearing the sticky state:

```c
RCC_CSR_REG |= (1 << 24);
```

### Hardware Fault Injection and Dead-Loop Simulation

To prove the safety utility of the watchdog, the system operates as a state-driven fault injection testbench split across two timing phases:

* **Phase 1: Healthy Steady-State (0 to 4000 ms):** The main execution loop services the watchdog counter periodically via `IWDG_kick()` while handling asynchronous telemetry streams. The watchdog countdown timer is continuously refreshed before a timeout occurs.

* **Phase 2:** Induced Software Lockup (4000+ ms): Upon hitting the 4-second mark, a software fault is injected by trapping the CPU core inside a permanent `while(1)` block. This simulates an unhandled firmware crash or infinite polling loop. Because the execution path is blocked, the reload key-register sequence is starved, causing the hardware timer to expire and trigger a reset exactly 2.0 seconds later.

## 📐 Mathematical Modeling & Timing Budget

The Independent Watchdog is driven by a dedicated, low-power internal Low-Speed Internal (`LSI`) RC oscillator operating at an uncalibrated nominal frequency ($f_{LSI}$) of **32 kHz**.

To establish a highly precise 2-second timeout window ($T_{out}$), the timing parameters are configured using a prescaler divider ($P_R$) and a 12-bit down-counter reload value ($R_L$):

$$\text{Timeout Window } (T_{out}) = \frac{\text{Prescaler } (P_R) \times \text{Reload Value } (R_L)}{f_{LSI}}$$

Given the target parameters chosen for this test bench:

* Prescaler Divider ($P_R$) = \mathbf{64}$ (Configured via IWDG_PRESCALER_64)
* Target Timeout ($T_{OUT}$) = **2.0** seconds
* Watchdog Clock Frequency = \frac{32{,}000\text{ Hz}}{64} = **500** Hz = **2** ms per clock tick

Solving for the required hardware Reload Value ($R_L$):

$$R_L = \frac{T_{out} \times f_{LSI}}{P_R} = \frac{2.0 \times 32{,}000}{64} = \mathbf{1000}$$

```c
watchdog.prescaler_val = IWDG_PRESCALER_64;
watchdog.reload_val = 1000;
```

## 🛠️ Peripheral Interfacing via Dynamic Memory Offsets

Instead of relying on hardcoded preprocessor register macro mappings, this driver interacts directly with the `IWDG` peripheral configuration space by executing 32-bit pointer arithmetic relative to a dynamic base register boundary address inside the configuration handle (`wdg->IWDG_BASE_REG`).

### 1. Pointer Dereference Mapping Matrix

Because pointers are defined as 32-bit types (`uint32_t*`), each integer increment moves the absolute target hardware address forward by exactly 4 bytes:

| **Peripheral Register** | **Offset** | **Pointer Assignment Syntax** | **Key Value / Configuration Purpose** |
| ---------| ----------- | ---------- | ------------- |
| **Key Register (`IWDG_KR`)** | `0x00` | `*(wdg->IWDG_BASE_REG + 0)` | `0xCCCC` (Start) / 0x5555 (Access Unlock) / 0xAAAA (Kick) |
| **Prescaler Register (`IWDG_PR`)** | `0x04` | `*(wdg->IWDG_BASE_REG + 1)` | Maps prescaler enum values (`0x00` to `0x07`) to clock dividers |
| **Reload Register (`IWDG_RLR`)** | `0x08` | `*(wdg->IWDG_BASE_REG + 2)` | Loads the 12-bit down-counter value (1000) |
| **Status Register (`IWDG_SR`)** | `0x0C` | `*(wdg->IWDG_BASE_REG + 3)` | Checked to ensure updates are fully processed by the LSI clock domain |

### 2. Defensive Asynchronous Synchronization Window

Because the `IWDG` peripheral lives completely within the asynchronous, low-frequency internal oscillator (`LSI`) clock domain, updating the prescaler or reload values requires up to 5 clock cycles to bridge across the internal bus domains.

The driver ensures structural resilience by masking out the status register with a fixed configuration bitmask (`0x03`) to isolate the **PVU** (Prescaler Value Update, Bit 0) and **RVU** (Reload Value Update, Bit 1) flags. A dedicated loop timeout boundary ensures the CPU core can safely bypass an oscillator fault if the hardware fails to clear the pending flags:

```c
volatile uint32_t timeout_counter = 100000;

// waiting until both update flags clear
while (((*(wdg->IWDG_BASE_REG + 3) & 0x03) != 0x00) && (timeout_counter > 0))
{
    timeout_counter--;
}
```

## 🚀 System Verification & Execution

### 1. Compilation and Deployment

Build the unified testbench using the existing workspace cross-compiler configuration and flash the binary image directly onto the microcontroller:

```bash
make clean && make
make flash
```

### 2. Live Telemetry Analysis

Connect a terminal adapter to pins `PA2` and `PA3` at **115200 baud** to view the real-time execution:

#### Cold Start (First Boot Cycle):

```text
--- MCU Boot Sequence ---
[INFO] System booted normally (Clear Power-On).
[INFO] Arming the Independent Watchdog...
[INFO] Watchdog Active. Running normal loop for 4 seconds.
System Status: OK (Kicking Dog)
System Status: OK (Kicking Dog)
System Status: OK (Kicking Dog)
System Status: OK (Kicking Dog)

!!! CRITICAL FAILURE: Software entry into infinite loop !!!
Stopping all watchdog kicks. Reset expected in 2.0s...
```

**Note:** The PA5 LED immediately shuts off, then the terminal pauses for exactly 2 seconds before the watchdog forces a hard reset.

#### Post-Watchdog Reset (Second Boot Cycle):

```text
--- MCU Boot Sequence ---
[ALERT] System recovered from a WATCHDOG RESET!
```

**Note:** The PA5 LED flashes rapidly 5 times as the firmware clears the `IWDG_RSTF` bit before resuming normal execution.

## 🗺️  Next Steps & Learning Roadmap

To transition this basic validation framework into an automotive-grade Functional Safety (`ISO 26262`) watchdog management strategy, upcoming implementations will explore:

1. **Windowed Watchdog Operation (WWDG):** Advance to the Window Watchdog peripheral, which forces a reset if the software kicks the dog too early (preemption fault) or too late, guarding against clock acceleration anomalies.
2. **Complementary Task Monitored Architecture**: Move beyond a simple main-loop kick by implementing an execution checkpoint matrix. Each independent software task must register its completion status before a global timer thread performs the hardware peripheral refresh.
3. **Low-Power Mode Interfacing**: Configure watchdog behavior during low-power sleep cycles (`Stop` or `Standby` modes) utilizing the flash option bytes configuration register to dictate whether the watchdog remains active or freezes during low-power states.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
