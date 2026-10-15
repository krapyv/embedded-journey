# Closed-loop DC motor speed controller

A closed-loop DC motor speed controller: TIM1 encoder -> TIM2 ISR (15 ms): speed + PI (Kp = 0.03, Ki = 0.5, Kd = 0 (unused)) -> TIM4 PWM + IN1/IN2 -> H-bridge motor driver TB6612FNG -> DC motor -> encoder.
The output clamping anti-windup on the integral.
It sits on top of the existing SysTick driver.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│    ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│    └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
│
├── periph/     # Portable peripheral drivers
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, clock and register configuration and function headers
│
├── main.c      # Application entry point: TIM1 encoder speed measurement, TIM2 15 ms control tick, TIM4 PWM output, PID update every 15 ms, motor direction  and duty control
├── app_config.h    # The header file for the PID struct: Kp, Ki, Kd, integral, prevMeasured, outMin and outMax for clamping
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
└── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
```

## Architectural Decisions:
 
1.  Under Kp = 0.03 the response is fast enough but starts to oscillate, so we added the Ki to eliminate remaining steady-state error. Initial values were Kp = 0.1 and Ki = 2, so we tuned them down.
Measured plant: K ≈ 160 RPM per unit of duty, deadband ≈ 0.05, τ ≈ 135 ms, 25 kHz PWM. Measured during the open-loop step.

## Test Results

u = duty output, integral measures in `RPM*s`, tick = one 15 ms sample.

1. The speed reaches the setpoint of 80 RPM with no steady-state error (80.2, within one encoder count of 1.04 RPM), about 0.33 s and 5.5% overshoot.

2. Disturbance load test (the finger on the wheel):
- Speed dips to 79 under load, recovers to 80 in 10 ticks, with a 3.3 RPM overshoot on that recovery.
- Overshoot peak - 83.3 RPM. 
- On release the speed jumped to 100, was back at 80 in 4 ticks, dipped to 75, and then settled by tick 12. 
- Afterwards `u` and `integral` returned to about their unloaded values.
- The extra 0.18 of duty came from the integral (0.5 * 0.32 = 0.16, plus a small P share).

3. Windup test:

With the windup guard (blocks when the output is pinned and the error pushed the same way: pinned at 1.0 with positive error, or pinned at 0 with negative error):
- The target dropped from 200 to 80 -> the speed fell at once from about 134, bottomed at 66.7 around samples 9-10, was within 2 RPM of 80 by sample 15 (0.23 s), and was settled by tick 17.
- `u` left 1.0 immediately.

Without the guard:
- A 44 RPM error adds about 0.66 `RPM*s` per tick (44 RPM * 0.015 s) to the integral, so the integral term alone holds the output at 1.0 for 53 ticks.
- The integral is frozen at 1.1 when saturation began.
