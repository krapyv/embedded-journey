# My Embedded Journey Log


> ⚠️ **Note to Readers/Recruiters:** This file is a raw, daily engineering notebook.
> I spend a strict maximum of 15-20 minutes each evening logging my raw notes, problems, and fixes. 
> The vocabulary here is informal and unpolished to maximize time spent writing actual production code.


## Overview
- Start date: April 23, 2026
- Goal: 24-month automotive embedded engineer
- Daily schedule: 4-4.5 hours (4:00 AM start)

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
 
