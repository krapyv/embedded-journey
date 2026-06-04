# My Embedded Journey Log


> ⚠️ **Note to Readers/Recruiters:** This file is a raw, daily engineering notebook.
> I spend a strict maximum of 15-20 minutes each evening logging my raw notes, problems, and fixes. 
> The vocabulary here is informal and unpolished to maximize time spent writing actual production code.


## Overview
- Start date: April 23, 2026
- Goal: 24-month automotive embedded engineer
- Daily schedule: 4-4.5 hours (4:30 AM start)

## Completed Milestones

## Daily Log

# 202x-xx-xx

**Morning:**
-

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Lesson learned:**
-

# 2026-06-04

**Morning:**
- Added a 104 (100 nF) ceramic capacitor in order to create RC filter to filter out mechanical switch bounces.
- Tested the circuit with a multimeter.
- Tested the program.
- Learned basic information about the retargeting system-layer functions.
- Made up the logs in the JOURNAL, describing all the events/details from the last 4 days.

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Lesson learned:**
-

# 2026-06-03

**Morning:**
- Fixed UART.c due to the changes in the .h file.
- Redirected standard C print functions directly to mycustom UART driver hardware (or so I thought).
- Assembled the electrical circuit with a tactile push button, the Blackpill, 10K Ohms resistor, an UART-to-USB adapter.
- Started to debug the program via ST-LINK.

**Evening:**
- Continued debugging.
- Simplified the TIM3_IRQHandler.
- Moved Update/Interrupt enablement to the end of the Input Capture init function.
- Wrote `_write` function to override a weak and empty toolchain stub.
- The program has been flushed and works correctly -> I still need to filter mechanical switch bounces.

**Problems encountered:**
- Well, the debug sessions were not pleasant.
- My misunderstanding of toolchain stubs and where I should enable Capture/Compare Interrupt and Update Interrupt (DIER) bits caused me several hard hours :)

**Lesson learned:**
- Embedded debugging (ST-LINK).
- Retargeting system-layer functions like `_write`.

# 2026-06-02

**Morning:**
- Integrated NVIC to the Input Capture.

**Evening:**
- Integrated UART to the Input Capture.
- Redefined UART.h as a driver file.

**Problems encountered:**
- Forgot to enable NVIC interrupts.

**Lesson learned:**
- Custom libraries integration.

# 2026-06-01

**Morning:**
- Implemented the basic functionality of input capture
- Developed and implemented input button timestamps capture.

**Evening:**
- Developed the input capture interrupt test function.
- Started implementing the NVIC configuration struct.
- Learned offset calculation and the basics of hex math.

**Problems encountered:**
- Ambiguity of offsets: i did not know why and how they are calculated.

**Lesson learned:**
- NVIC register map.
- Offset calculation.
- HEX math.


# 2026-05-31 

**Morning:**
- Finished the PWM LED Dimmer.

**Evening:**
- Learned basic information about Input Capture and why do I need.

**Problems encountered:**
- Understanding of hardware mapping structs vs configuration handles;

**Lesson learned:**
- TIM hardware registers.


# 2026-05-30

**Morning:**
- Finished reading and fixing typos in the READMEs.md.
- Defined the hardware mapping structs for the LED Dimmer

**Evening:**
- Continued to work on the LED Dimmer

**Problems encountered:**
- None

**Lesson learned:**
- Basic aspects of PWM (Pulse Width Modulation).

# 2026-05-28, 2026-05-29

- I had been writing READMEs.md for my newest projects.

**Problems encountered:**
- Documentation is hard. It is brutal. But there is no way to omit it.

**Lesson learned:**
- Docs suck. But you still need to write them.


# 2026-05-27

**Morning:**
- Figured out that the LED is working perfectly at my breadboard since the circuit supplies between 1.3 mA and 5.9 mA to the LED. The LEDs in the kit I bought are rated all the way up to 20 mA for absolute maximum brightness, so running it at 1.3 mA - 5.9 mA means I am operating it in its power-saving indicator range.
- Debugged and tested IWDG library in combination with LED, ring buffer and UART libraries. 

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Lesson learned:**
-

# 2026-05-26

**Morning:**
- Learned basics about IWDG (Independent Watchdog).
- Implemented a reusable IWDG library.

**Evening:**
- Tested and debugged UART polling transmitter with an USART-to-USB adapter.
- Started to test and debug UART interrupts project, encountered some problems with LEDs (half-bright).

**Problems encountered:**
- An LED brightly lights only at the top. Spent several hours to figure out what caused it.

**Lesson learned:**
- IWDG.
- Testing and debugging of UART.

# 2026-05-25

**Morning:**
- Finished UART interrupts.
- Implemented handling of ORE and FE errors.
- Implemented an atomic critical section in main to prevent race conditions.
- Implemented fully reusable architecture independent LED library.

**Evening:**
- Learned basics of the assembly startup file.

**Problems encountered:**
- It was quite hard to grasp all that race conditions stuff. But once understood, it started to make sense.

**Lesson learned:**
- ORE and FE error flags.
- Atomic critical sections and race conditions.
- Hardware abstraction layers.
- Startup files .s.

# 2026-05-24

**Morning:**
- Learn basic information about circular/ring buffers.
- Developed, implemented and tested an architecture-independent ring buffer in C.

**Evening:**
- Learned the foundation theory of UART Receivers;
- Decoded the ARM Cortex-M4 Core Architecture (mapped out the NVIC_ISERx) bit-assignment structure;
- Integrated an Interrupt-Driven UART Driver.

**Problems encountered:**
- Too compressed manuals (e.g. for the ARM Cortex-M4). I realised that you need to decode what is in it, because not everything is written explicitly.

**Lesson learned:**
- Ring/circular buffers;
- UART Receiver;
- ARM Cortext-M4 NVIC.

# 2026-05-23

**Morning:**
- Learned the basics of UART communication (Baud rate, frames, how data travels without a clock wire).
- Wrote a bare-metal program to configure USART2 on the Black Pill for transmitting data at 115200 baud.

**Evening:**
- Wanted to print an output of UART transmission to an OLED display, started writing the functionality to do so, but decided to hit pause on that to the moment I would learn I2C.

**Problems encountered:**
- STLINK/V2 does not have RX/TX ports (virtual COM port). Cannot test my UART transmit without it or an adapter. Ordered one.
- I2C is way more complex than UART/USART.

**Lesson learned:**
- Go next with OLED screens for the UART transmit when the time comes during the I2C topic (several topics from now).
- UART is a quite powerful protocol.

# 2026-05-22

**Morning:**
- Mastered the theory behind SysTick interrupts and ARM core exception vector handling (TICKINT and SysTick_Handler);
- Rewrote a busy-wait timer in the Button-Controlled LED Blink program with a non-blocking background hardware timer heartbeat (1 ms tick rate);
- Created an LED Blink program that toggles the LED every 10 button presses using edge detection and non-blocking timer driven by interrupts.

**Evening:**
-

**Problems encountered:**
- Non-blocking timers is a hard topic to understand, but I took my time to grasp its fundamentals.

**Lesson learned:**
- SysTick Interrupts;
- Static variables.


## 2026-05-21

**Morning:**
- Wrote a README.md for the Button-Controlled LED Blink project;
- Analyzed my overall progress;
- Reviewed remaining tasks in 24-month plan
- Started learning SysTick (polling)

**Evening:**
- Rewrote a busy-wait timer in the LED Blink program with a busy-wait countdown timer using SysTick. Now my delay function is more reliable and professional.

**Problems encountered:**
- it was quite hard to grasp the concept of SysTick internal hardware decrementing of the current value register (SYST_CVR). Leaving empty `while` loops feels weird at first.

**Lesson learned:**
- Documentation is hard to write and boring to structurize, but it is as important as code

## 2026-05-20

**Morning:**
- Button controlled LED blink working on hardware
- Integrated button debouncer to the project
- Fixed split power rail issue

**Evening:**
- Wrote the README.md for LED Blink Bare Metal program;
- Started writting the README.md for Button-Controlled LED Blink

**Probles encountered:**
- Pull-up resistor not connected to powered rail
- Button not connected to grounded rail

**Lesson learned:**
- Breadboard power and ground rails are split; must connect top and bottom.


### Past is to be added
 
