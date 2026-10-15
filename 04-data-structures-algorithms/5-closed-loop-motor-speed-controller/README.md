# Closed-loop DC motor speed controller

A closed-loop DC motor speed controller: TIM1 encoder -> TIM2 ISR (15 ms): speed + PI (Kp = 0.03, Ki = 0.5, Kd = 0 (unused)) -> TIM4 PWM + IN1/IN2 -> H-bridge motor driver TB6612FNG -> DC motor -> encoder.
Anti-windup: the integral stops accumulating while the output is saturated and the error pushes further into saturation. Output is clamped to [0, 1].
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
├── main.c      # Application entry point: TIM1 encoder speed measurement, TIM2 15 ms control tick, TIM4 PWM output, PID, motor direction and duty control
├── app_config.h    # The header file for the PID struct: Kp, Ki, Kd, integral, prevMeasured, outMin and outMax for clamping
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
└── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
```

## Architectural Decisions
 
1. Kp = 0.03: a gain low enough (K * Kp = approx. 4.8, ringing factor positive = +0.36) to be safely stable with one tick of measurement delay, chosen from the loop-gain formula and confirmed by a clean P-only step.
Ki = 0.5: the Stage 0 (software plant derivation) value, chosen to remove the P-only steady-state error within about 0.3 s, and confirmed on the real motor with 5.5% overshoot.
Measured plant: K ≈ 160 RPM per unit of duty, deadband ≈ 0.05, τ ≈ 135 ms, 25 kHz PWM, supply voltage = 6 V. Measured during the open-loop step, 3840 counts per revolution (measured over 10 revolutions).

2. The control loop is running in the TIM2 ISR, not in the main loop.
The TIM2 ISR fires every 15 ms, preempting the main loop. No matter what the main loop is doing, the TIM2 interrupt fires and the control loop runs within interrupt latency.
15 ms is short enough time to track the plant and not too rapid for the ISR to fire non-stop.

## Test Results

u = duty output, integral is measures in `RPM*s`, tick = one 15 ms sample.

1. The speed reaches the setpoint of 80 RPM with no steady-state error in about 0.33 s (80.2, within one encoder count of 1.04 RPM), 5.5% overshoot.

2. Disturbance load test (the finger on the wheel):
- Speed dips to 79 under load, recovers to 80 in 10 ticks, with a 3.3 RPM overshoot on that recovery.
- Overshoot peak - 83.3 RPM. 
- On release the speed jumped to 100, was back at 80 in 4 ticks, dipped to 75, and then settled by tick 12. 
- Afterwards `u` and `integral` returned to about their unloaded values.
- The extra 0.18 of duty came from the integral (0.5 * 0.32 = 0.16, plus a small P share).

3. Windup test:

With the windup guard (blocks when the output is pinned and the error pushed the same way: pinned at 1.0 with positive error, or pinned at 0 with negative error):
- The target dropped from 200 to 80 -> the speed fell at once from about 134, bottomed at 66.7 around ticks 9-10, was within 2 RPM of 80 by sample 15 (0.23 s), and was settled by tick 17.
- `u` left 1.0 immediately.
- The integral is frozen at 1.1 when saturation began.

Without the guard (calculated):
- A 44 RPM error adds about 0.66 `RPM*s` per tick (44 RPM * 0.015 s) to the integral, so the integral term alone holds the output at 1.0 for 53 ticks.

## Breadboard Circuit

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

IMPORTANT: make sure you limited the XL4015 current to 1 A and 6 V voltage.

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

## Known Limitations

1. The PID lives in `main.c`, not in a reusable component.
2. No encoder-loss or stall detection: if the cable falls off, the loop reads 0 RPM and drives `u` to 1.0.
3. The controller assumes the encoder counts up when the motor turns forward. If the motor wires or encoder A/B are swapped, measured speed is negative, the error never closes, and `u` pins at 1.0.
4. The controller is PI - Kd is unused but it is in the struct (Kd = 0).
5. Reverse is in open-loop only, closed-loop goes only forward (output clamped to [0,1]).
