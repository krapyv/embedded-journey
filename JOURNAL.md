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

**Probles encountered:**
- (None today) etc

**Lesson learned:**
-

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
 
