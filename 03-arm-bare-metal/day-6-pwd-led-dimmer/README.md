# PWM Led Driver

This project implements a reusable, register-level, hardware-agnostic Pulse Width Modulation (PWM) peripheral driver for STM32F411 microcontroller. Operating entirely bare-metal without relying on bloated abstraction layers, the driver allows any general-purpose timer (`TIM2` to `TIM5`) to modulate an LED signal on any compatible General Purpose Input/Output (`GPIO`) pin.

The accompanying testbench application demonstrates a localized, non-blocking breathing loop on the onboard user indicator LED (Pin `PA5` via `TIM2_CH1`), validating edge-aligned signal generation, alternate-function multiplexing, and dynamic capture/compare register modification. 

## 📂 Project Structure

```text
├── main.c              # Application logic, configuration profiles, and the duty-cycle breathing loop
├── pwm_driver.c        # Multi-channel timer clock gating, Alternate Function routing, and CCMR configuration
├── pwm_driver.h        # Driver configuration handles, register shift helpers, and function abstractions
├── stm32f411.h        # Core memory map definitions, peripheral boundaries, and explicit register structs
├── startup_stm32f411ceux.s       # Assembly startup file implementing vector table & Reset_Handle   
├── stm32f411.ld  # Linker script defining Flash/SRAM memory segments
└── Makefile      # GNU Make automation configuration to compile and flash the system
```

## 🧠 Core Low-Level Concepts
### Hardware Alternate Function Multiplexing

To route an internal hardware timer channel to a physical silicon pin, the GPIO pin must be decoupled from the standard data registers (`IDR`/`ODR`) and mapped to the chip's internal digital matrix using Alternate Function configurations.

* **Mode Selection:** The GPIO Mode Register (`MODER`) must be set to `0x2` (Alternate Function Mode) for the target pin.

* **Low vs. High Pin Split:** The alternate function mapping is divided between two 32-bit registers: `AFRL` (Pins 0-7) and `AFRH` (Pins 8-15). Each pin takes up exactly **4 bits** within these registers to hold a 4-bit alternate function index (`GPIO_AF0` to `GPIO_AF15`).

* **Bitwise Safety Math:** The driver implements automatic index scaling:

$$\text{Bit Shift Offset (Pins 0--7)} = \text{pin} \times 4$$

$$\text{Bit Shift Offset (Pins 8--15)} = (\text{pin} - 8) \times 4$$

```c
pwm->Port->AFRL &= ~(0xF << (pwm->pin * 4));
pwm->Port->AFRL |= (pwm->af << (pwm->pin * 4));

pwm->Port->AFRH &= ~(0xF << ((pwm->pin - 8) * 4));        
pwm->Port->AFRH |= (pwm->af << ((pwm->pin - 8) * 4));
```

### Dual-Register Capture/Compare Mode Control (`CCMR1`/`CCMR2`)

Timers on the STM32 architecture share register configurations in pairs to maximize space. The Output Compare Mode channels are controlled across two registers:

* `CCMR1`: Manages configurations for **Channel 1** and **Channel 2**
* `CCMR2`: Manages configurations for **Channel 3** and **Channel 4**

The driver handles the asynchronous offset shifting dynamically using a ternary check based on channel parity. Even channels (2 and 4) map their configuration fields to bit position `12`, while odd channels (1 and 3) map to bit position `4`:

```c
uint32_t pos = ((pwm->channel & 1) == 0) ? 12 : 4;
```

Within these positions, a 3-bit code configuration field sets the specific output behavior. The driver enforces **PWM Mode 1 (`0x6` or `0b110`)**. In up-counting mode, this configuration forces the channel output pin to remain high (`1`) as long as the counter register (`CNT`) is strictly less than the capture/compare value (`CCR`), dropping low (`0`) for the reminder of the period loop.

## 📐 Mathematical Modeling & Timing Budget

The physical frequency ($f_{PWM}$) and the resolution of the breathing effect are governed directly by the clock domain supplying the target timer instance. Driven by the internal High-Speed Internal (`HSI`) oscillator running at a native **16 MHz**, the timer operates without a pre-scaler division factor ($PSC = 0$).

The application creates a 1 milisecond period framework ($T_{PWM} = 1 ms$) by loading the Auto-Reload Register (`ARR`) with the total calculated tick counts required to consume that duration:

$$T_{PWM} = \frac{\text{Auto-Reload Value } (ARR) + 1}{f_{Timer\_Clock}}$$

$$1\text{ ms} = \frac{ARR + 1}{16{,}000{,}000\text{ Hz}} \implies ARR = 15{,}999$$

The Duty Cycle ratio ($D$) represents the proportion of time the LED remains on during a single period. This value is modulated continuously inside the application loop by writing to the corresponding index inside the CCR structure array:

$$D = \frac{\text{Capture/Compare Value } (CCR)}{ARR + 1}$$

```c
pwm.Period = 15999;
pwm.Pulse = 8000;
```

## 🛠️ Unified Register Mapping Architecture

This framework relies on structural layout overlays to interact with memory-mapped registers. Because the structure mappings exactly mirror the memory geometry of the microcontroller's hardware, the multi-channel `CCR` configurations are represented as a contiguous array of 32-bit types (`uint32_t CCR[4]`).

### Structure Field Alignment

```c
typedef struct
{
    volatile uint32_t CR1;       // 0x00
    volatile uint32_t CR2;       // 0x04
    volatile uint32_t SMCR;      // 0x08
    volatile uint32_t DIER;      // 0x0C
    volatile uint32_t SR;        // 0x10
    volatile uint32_t EGR;       // 0x14
    volatile uint32_t CCMR1;     // 0x18
    volatile uint32_t CCMR2;     // 0x1C
    volatile uint32_t CCER;      // 0x20
    volatile uint32_t CNT;       // 0x24
    volatile uint32_t PSC;       // 0x28
    volatile uint32_t ARR;       // 0x2C
    volatile uint32_t RESERVED0; // 0x30
    volatile uint32_t CCR[4];    // 0x34,0x38, 0x3C, 0x40
    volatile uint32_t RESERVED1; // 0x44
    volatile uint32_t DCR;       // 0x48
    volatile uint32_t DMAR;      // 0x4C
    volatile uint32_t OR;        // 0x50 (only TIM2 and TIM5)
} TIM_RegDef_t;

```

By standardizing on a dedicated configuration structure handle (`PWM_HandleTypeDef`), an application developer can switch the entire physical pin assignment or timebase profile simply by initializing a new structural handle profile:

```c
typedef struct
{
    TIM_RegDef_t *Instance; // timer
    GPIO_RegDef_t *Port;    // gpio
    uint32_t pin;
    GPIO_AF_t af;
    uint32_t Period;
    uint32_t Pulse; // duty cycle
    uint32_t channel;
} PWM_HandleTypeDef;
```

## 🚀 System Verification & Execution

### 1. The Breathing LED Implementation Algorithm

The application achieves a smooth "breathing" illumination loop by adjusting the LED's duty cycle over time using a state delta tracker variable (`step`). The value increments or decrements linearly every cycle:

```c
 while (1)
    {
        brightness += step;

        if (brightness >= 15000 || brightness <= 500)
        {
            step = -step;
        }

        pwm.Instance->CCR[pwm.channel - 1] = brightness;

        for (volatile int i = 0; i < 100000; i++)
            ;
    }
```

### 2. Compilation and Target Flash Deployment

Compile the reusable code tree using standard cross-compilation toolchain (`arm-none-eabi-gcc`) and upload the binary image using GNU Make automation commands:

```bash
# wipe out previous compilation symbols and build executable image
make clean && make

# deploy the binary safely down to the target microchip memory
make flash
```

Once flashed successfully, the hardware pin `PA5` will drive the onboard LED through a non-blocking breathing cycle, operating completely independently of any CPU intervention beyond the `CCR` register adjustments.

## 🗺️  Next Steps & Learning Roadmap

To build on this bare-metal foundation and scale it toward a comprehensive automotive engine or chassis control framework, upcoming iterations will implement:

**1. Dead-Time Insertion & Complementary Output Generation:** Implement dual-channel complementary PWM configurations via H-bridge topologies to safely drive electric motor powertrains without causing short circuits.

**2. DMA-Driven Duty Cycle Arrays:** Offload the CPU from handling the breathing math loops entirely by streaming lookup arrays directly into the timer's `CCR` register space via Direct Memory Access (`DMA`) streams driven by an input trigger.

**3. Hardware Input Capture Optimization:** Build out a companion `input_capture.c` driver module. This will leverage the timer's alternate input channels to latch edge transitions, allowing the chip to decode external PWM signals from incoming sensors.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
