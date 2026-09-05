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

**Evening:**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-05

**Morning:**
- Learned about why does a bootloader exist and a usage of bootloaders in automotive industry.
- Finished developing (designing) the UART chunk-receive protocol.
- Designed the GPIO-pin check.

**Evening:**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-04

**Morning:**
- Finished debugging the Flash program and erase functions.
- Implemented the Flash error checking function.
- Started design the UART chunk-receive protocol: chunk size, framing - what a single packet actually does need to contain.
- Learned about read and write directly from/to a specific hardware memory address (`*(uint32_t*)(addresses[i])`).

**Evening:**
- Continued to design the UART chunk-receive protocol. Almost done: there is still ongoing SysTick timeout derivation.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-03

**Morning:**
- Finished designing the bootloader.
- Implemented the FLASH register map struct, the address define macro and the useful Flash enums.
- Wrote a Linkedin post about the pre-work I have had before even touching the bootloader development.

**Evening:**
- Implemented the Program and Erase Flash functions and debugging them.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-02

**Morning:**
- Learned about LDR and STR.
- Learned about MSP and PSP as well as their differences and PSP in RTOS.
- Theory is finally almost over (the last thing is SP and PC). The designing of the bootloader is next.

**Evening:**
- Learned about SP and PC.
- Started designing the bootloader.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-01

**Morning:**
- Finished the stack pointer validation.
- Learned the Stack Pointer Alignment.
- Learned about naked function attribute (`__attribute__((naked))`).
- Learned about the C function prologue and epilogue.
- Learned about the naked function vs normal C function.
- Started learning and deriving the Bare-bones HardFault handler (register dump on fault, halt).

**Evening:**
- Repeated the Full Descending stack model.
- Finished working on the bare-bones HardFult handler (re-read of the last derivations is needed).

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-31

**Morning:**
- Learned about the vector table relocation - SCB_VTOR register, why the application's vector table must be at a known offset. Designed it for the upcoming bootloader application.
- Started learning about the flash sector layouts for the upcoming bootloader application.

**Evening:**
- Finished learning about the flash sector layouts.
- Started working on stack pointer validation and why first word of application vector table must be a valid SRAM address before jumping.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-30

**Morning:**
- Finished learning the LD linker script: stack model, .data/.bss copy semantics, alignment hazard mechanics, all understood at the register and byte levels.
- Learned about the Startup file and its sections.

**Evening:**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-29

**Morning:**
- Finished repetion of I2C and SPI with questions.
- Learned about the LD linker script. Next is understanding what is happening and being a bit more comfortable with all of this.

**Evening:**
- Continued to trying to grasp the GNU LD linker script.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-28

**Morning:**
- Measured once again, eliminated all possible reasons why the measurement results have significant excesses.
- Updated the UART driver: both the submodule and the local copies in the main repo.
- Wrote a LinkedIn post about the measurements of the Low Power modes project.

**Evening:**
- Repeated the I2C main information and stuck at some questions.

**Current measurement results:**
- Active: 7.1mA (predicted 3.6-4.8mA typ per RM0383 Table 24 - some excess likely from real peripheral usage beyound the "disabled" baseline row).
- Sleep: 3.4mA (clear drop from active, consistent with CPU-halt-only behavior).
- Stop: 2.7mA (predicted ~43µA typ - significant unexplained excess).
- Standby: 0.6-0.7 (predicted ~2.1-4µA typ - significant unexplained excess).

Investigated and ruled out as sole causes: external pull-up resistor (removed, no change), ST-Link connection (disconnected, not change), USART-USB adapter (disconnected, no change). Confirmed contributing factor: onboard PWR LED (4.7kΩ series resistor) draws ~0.3mA continuously regardless of MCU power state - accounts for a fraction of the gap but not all of it.
I ≈ (3.3V - V_LED_forward) / 4.7kΩ.
A typical LED forward voltage is roughly 1.8-2.2V (varies by color/type).
So I ≈ (3.3 - 2.0) / 4700 ≈ 1.3V / 4700Ω ≈ 0.28mA ≈ 0.3mA.
Remaining discrepancy source unidentified; not investigated further to avoid physically modifying/damaging the board.
All three modes confirmed functionally correct: relative current ordering (Active > Sleep > Stop > Standby) matches expected behavior, and wake mechanisms (SysTick, EXTI0, WKUP) all verified working via UART trace and reset-detection logic. 
The Done bar has been reached: the modes demonstrably work, current drops in the correct relative order, I understand why the absolute numbers do not match the datasheet.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-27

**Morning:**
- Checked my README.md for the Low-power modes project.
- Learned about fflush() and fflush(stdout) with my implementation of `_write()` that makes it an effective no-op.
- Learned about voltage regulator and low power voltage regulator.
- Learned about Hardware block diagrams and how read them.
- Learned about masked and non-masked interrupts.
- Remind myself about UART 8N1 configuration.
- Learned about the NVIC seeing the pending interrupt signal immediately upon unmasking IMR, even without a new edge occurring.
- Learned about CPSIE back-to-back with WFI is an explicitly supported, architecturally recognized sequence in ARM Cortex-M cores.

**Evening:**
- Started measuring the low power modes project: made a setup, tested the modes, started debugging.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-26

**Morning:**
- Ordered electrical tape and alligator clips for the Low Power measurements.
- Planned tomorrow's Low Power Mode measurement experiment.
- Completed the JOURNAL logs from 21.08 to 25.08.

**Evening:**
- Wrote a README.md for the Low-power modes project.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-25

**Morning:**
- Learned about POR/PDR (Power-On reset and Power-Down reset).
- Learned about the TXE vs TC USART race condition.
- Wrote a LinkedIn post about the PDDS bug in the Stanby mode branch implementation.

**Evening:**
- Tried to test measurements of Sleep, Stop and Standby. Realiazed the USB Current measurement is for min 10 mA, while the board in Active run is 5 mA. Do not have wire cutters and electrical tape either. Deferred for a bit.

**USB cable VBUS splice procedure for µA current measurement:**
- Cut only the red (VBUS) wire - leave GND, D+, D- untouched.
- Strip ~5mm from both new ends.
- Red probe to source/laptop side, black probe to board side, meter in series across the gap.
- For Active/Sleep: dial to A mA, red probe in left jack (FUSED 10A MAX).
- For Stop/Standby: dial to µA, red probe in right jack (VΩHz µA, 200mA MAX -fragile path).
- Switch jack and dial while board is powered down.
- Power up only after meter is correctly connected and correct range is selected.
- Let each reading settle 2-3 seconds before recording.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-24

**Morning:**
- Debugged the Sleep and Stop modes.

**Evening:**
- Debugged the UART timing TC vs TXE bug.
- Debugged the Standby mode. Fixed all the bugs. Now the program works.

**Measurement equipment plan established:**
- USB inline meter (10mA resolution): Active vs Sleep comparison. Sufficient to demonstrate visible drop; not capable of resolving µA-range currents.
- ANENG AN8009 in DC µA mode (99.99µA range, 0.01µA resolution), red probe in VΩHz µA jack (200mA MAX - fragile path), in series on cut VBUS red write: Stop and Standby measurement.
- Jack/dial pairing discipline: mA/A range -> left jack (FUSED 10A MAX); µA range -> right jack. Never mismatch. Always power down before switching jacks between measurement phases.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-23

**Morning:**
- Developed the Standby mode branch.
- Implemented the Standby mode branch.
- Fixed my own bugs in the implementation.

**Evening:**
- Built the breadboard circuit for the project. Tested it.

**Architectural derivation - Standby mode exit behavior:**
Standby mode powers down the entire voltage regulator, not just clocks. SRAM and register contents - including the stack pointer, global variables, and the entire execution context - are lost. On PA0 rising edge, the MCU does nit resume from the instruction after WFI(). It resets entirely, executing from the reset vector as if it had just powered on.

Consequence for the UART menu: "woke from Standby" is not a returning code path. It is a full re-init. The firmware needs to detect, at the very top of main() before any peripheral initialization, that the wakeup reason was Standby - not a cold power-on - and print the confirmation message accordingly.

The register that persists across Stanbdy is PWR_CSR. Bit 0 is WUF (Wakeup Flag) - set when a wakeup even occurred. It must be explicitly cleared by writing bit 2 (CWUF) in PWR_CR. If not cleared, WUF remains set and is indistinguishable from a fresh wakeup on every subsequent power-on.

SBF (Standby Flag, bit 1 in PWR_CSR) tells you specifically that the wakeup was from Standby rather than a regular reset. Check SBF at the top of main(), print the confirmation, then clear it via CSBF (bit 3 in PWR_CR) before proceeding with normal init.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-22

**Morning:**
- Fully implemented the Stop mode branch, fixed the bugs and derived every line of code.

**Evening:**
-

**Bugs encountered and fixed:**

1. SYST->CSR |= ~(1UL << 0U) - inverted mask with OR sets everything except bit 0
**Root cause:** ~(1UL << 0U) = 0xFFFFFFFE. OR-ing this into CSR sets TICKINT, CLKSOURCE, and every reserved bit to 1. Bit 0 (ENABLE) is completely untouched - counter keeps running. Not a disable at all.
**Fix:** SYST->CSR &= ~(1UL << 0U) - AND with inverted mask clears only bit 0.

**Architectural derivation - EXTI_PR/IMR/WFI ordering for Stop mode:**
- EXTI_PR is set by the edge-detect circuit independently of EXTI_IMR. Edge detect -> OR gate -> Pending Register. IMR sits in an AND gate downstream of PR, gating whether PR propagates to the NVIC - not whether PR gets set.
- Consequence: unmasking IMR while PR=1 (from a stale or bounce edge) causes the AND gate to immediately forward the pending signal to NVIC - even with no new edge occurring. This is a combinational, not edge-triggered, check.
- Correct register write order: SYSCFG_EXTICR1 -> EXTI_FTSR -> NVIC_ISER -> EXTI_IMR -> EXTI_PR clear -> `__WFI()`.
- IMR must be stable (unmasked) before the final PR clear, not after - otherwise unmasking IMR onto a still-dirty PR can fire the interrupt mid-configuration.
- PR clear must be the very last register write before WFI - minimizes the vulnerable window, but cannot close it to exactly zero in software. This is an inherent hardware race that can only be minimized, not eliminated.
- PRIMASK (cpsid i) is not the solution: it sits downstream of EXTI's edge-detect logic (core-level, not peripheral-level), so it cannot prevent PR from being set by hardware. Futhermore, WFI wakes on "a PRIMASK-masked interrupt becomes pending" - so a stale edge during a PRIMASK-protected window still causes immediate WFI return, making it worse that useless for this hazard.
- CPSIE I immediately before WFI is the correct pattern: CPSIE takes effect on the next instruction boundary, so CPSIE -> WFI is atomically safe - the wake condition becomes active exactly as WFI begins.

**Problems encountered:**
- The Stop entry sequence was so hard to grasp because of the races `__WFI` location :)

**Root cause at the register level:**
-

# 2026-08-21

**Morning:**
- Implemented the Sleep mode branch of the project. The debugging is next.

**Evening:**
- Ordered the USB current meter.
- Finished the Sleep mode branch.
- Designed the Stop mode branch.

**What was done:**
- Implemented and debugged Sleep mode on STM32F411. Designed the full architecture: SysTick-driven 1ms timebase as wake source, ring buffer for UART RXNE decoupling, inspect_byte() function shared between menu selection and Sleep loop exit check, exit_button pressed flag ownership assigned to ISR only.
- Designed Stop mode EXTI0 interrupt configuration sequence. Derived correct register write ordering and resolved the race window between EXTI_PR clear and WFI entry. Established measurement equipment plan: USB inline meter (10mA resolution) for Active/Sleep comparison, ANENG AN8009 in µA mode on cut VBUS wire for Stop/Standby.

**Bugs encountered and fixed:**

1. SCB->SCR |= ((0 << 4) | (0 << 2)) - OR-with zero is a silent no-op.
**Symptom:** SLEEPDEEP and SEVONPEND not actually being cleared.
**Root cause:** (0 << 4) | (0 << 2) evaluates to 0x00000000. OR-ing any register with 0 leaves all existing bits unchanged. If SLEEPDEEP had been set by a previous Stop mode entry, this line would silently fail to clear it, causing Sleep mode to accidentally behave like Stop.
**Fix:** SCB->SCR &= ~((1 << 4) | (1 << 2)) - RMW with inverted mask. SCB->SCR in an ordinary control register, not a write-1-to-set/clear register like NVIC ISER/ICER. It has no hardware protection against destructive RMW. Was harmless by coincidence (bits reset to 0 on boot, nothing else touched SCR yet), but latently broken for when Stop mode adds SLEEPDEEP=1.

2. SysTick_Init() inside the Sleep while loop - reinitializing SysTick every iteration
**Root cause:** CVR write-to-zero causes immediate reload and restart of countdown. With ENABLE already set from a previous init call, the counter restarts fresh every ~1ms, distorting the intended cadence.
**Fix:** Move SysTick_Init() above the while loop - initialize once, let it free-run.

3. exit_button_pressed never reset between Sleep entries
**Root cause:** Global flag set to 1 on exit, never cleared. Second call to process_sleep_mode() finds while (exit_button_pressed != 1) already true on entry - loop body never executes, Sleep mode never engages.
**Fix:** Reset exit_button_pressed = 0 at the top of process_sleep_mode() before the loop. No race: ISR can't fire before UART is active, and process_sleep_mode() is the sole reset writer.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-20

**Morning:**
- Finished learning theory about the low-power modes.
- Designed the three modes via USART menu program.

**Evening:**
- Started developing a Sleep mode entry sequencing.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-19

**Morning:**
- Finished extracting the drivers and projects and submodule them between each other and the main repo. 
- Fixed several Makefiles and tested them.
- Completed the JOURNAL logs about the repo architecture migration.

**Evening:**
- Fixed READMEs.md for the extracted drivers
- Started learning about the low power modes.

**Repo architecture migration:**
- Extracted Week 21 integration project into its own portfolio repo stm32-can-sensor-node, excluding build artifacts (.elf/.bin/compile_commands.json) from the extraction entirely rather than including-then-deleting.
- Re-linked embedded-journey itself: removed original driver folders, re-added all 13 as submodules at original paths, verified via two independent fresh git clone --recurse-submodules passes that every nested dependency (up to 3 levels deep) resolves correctly.

**Submodule pitfalls hit and resolved:**
- Stale submodule pointer: fixing an include path in a standalone driver repo and pushing does NOT automatically update parent repos that already submoduled it - parent still points at the old pinned commit. Hit this 3x (i2c -> systick staleness surfaced via uart's ring_buffer missing entirely in embedded-journey). Fix: cd <submodule>; git checkout main; git pull; cd ..; git add <submodule>; git commit to bump the pointer.
- git submodule update --remote --merge updates which commit a submodule points to but does not reliably auto-populate newly appearing nested submodules within that update - always follow with git submodule update --init --recursive to be safe.
- Directory-navigation mixup during batch cloning (cloned two repos as subfolders of an unrelated submodule instead of siblings).

**Makefile migration to submodule structure:**
- Old SHARED_CORE_DIR flat-path model broken by nested submodule folders (e.g. i2c.c now at periph/i2c/i2c.c, not periph/i2c.c).
- Duplicate-symbol trap identified: same driver's .c file physically exists in multiple nested submodule copies (systick.c appears under mcp2515/systick/, bmp280/systick/, bmp280/i2c/systick/, and top-level systick/ - all byte-identical). Compilling more than one copy into SRC causes multiple-definition linker errors. Fix: compile exactly one .c per unique driver regardless of how many nested copies exist; C's quote-include search (relative to including file's own directory) resolves nested header dependencies automatically with no extra -I flags needed.
- Rewrote and verified Makefiles for stm32-can-sensor-node, day-9-i2c-temperature-pressure-polling, day-11-can against new submodule paths - all three compile clean.
- Deferred: remaining historical day-X Makefiles left un-migrated - noted as intentional decision, not oversight.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-18

**Morning:**
- Finished rewriting the USART README.
- Completed the JOURNAL logs for 15.08, 16.08 and 17.08.
- Wrote a Linkedin post about the Integration Project.

**Evening:**
- Extracted all drivers into separate repos and started submodule everything.

**Repo architecture migration - driver extraction into standalone repos:**
- Extracted 13 drivers/utilities from monolitic embedded-journey into standalone repos using git filter-repo, preserving full commit history per driver (not squashed/flattened).
- Built dependency-layered submodule graph based on actual #include audit - caught i2c depending on systick, bmp280 depending on i2c + systick, mcp2515 depending on core + spi + systick.
- stm32f411-core (stm32f411.h, core_cm4.h) submoduled into every driver that needs register/NVIC definitions - single source of truth, no duplication.
- Discovered and fixed a git filter-repo --path-rename bug: trailing-colon-no-destination (oldpath:) silently drops single-file blobs instead o renaming them - only works correctly for directory-prefix renames (olddir/:). Fixed by specifying explicit destination filename (oldpath:newname).

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-17

**Morning:**
- Build the breadboard circuit.
- Tested the USART DMA TX.
- Almost finished rewriting the USART README.

**Evening:**
-

**Program verification:**
1. tx_hit_counter == 1 after transfer. ISR fired exactly once, cleanly. Confirmed via GDB.
2. DMA1->S6NDTR == 0 after transfer. Hardware's own decremented count of remaining items confirms all 64 were moved. This is a stronger signal then terminal output.
3. minicom ASCII output displayed 0x20-0x3F printable range correctly (space through ?). Content correctness confirmed - byte order and completeness verified visually for the printable tail of the buffer.

**Cleanup after verification:**
1. raw_tx_buffer 0x30-0x3F population removed from uart.c - test scaffolding, not driver code.
2. tx_hit_counter retained as permanent instrumentation.
3. TODO comment left in DMA1_Stream6_IRQHandler marking the SR TC deferred path.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-16

**Morning:**
- Fully designed the DMA USART TX path: register inventory, write-order constraint, the TCIE/EN race condition, the TCIF6 vs SR-TC distinction, function boundary.
- Implemented the DMA TX initialization function and DMA1_Stream6_IRQHandler.

**Evening:**
- Developed the test bench main.c for USART DMA TX testing.

**Bugs encountered:**
1. DMA1_Stream6_IRQHandler re-entering indefinitely after the first TC fire.
**Symptom:** tx_hit_counter climbing continuously in GDB, DMA not stopping after 64 bytes.
**Root cause:** TCIF6 in HIFCR not cleared before ISR returned. HIFCR is a write-1-to-clear register. Because the flag was never cleared, the NVIC's pending bit re-latched from the still-asserted interrupt request line, and the ISR re-entered immediately on ever return. This is not recursive - the ISR returns fully to the NVIC before re-entering, so the stack does not grow. But it re-triggers at interrupt rate, functionally blocking main().
**Fix:** Direct wire DMA1->HIFCR = (1U << 21U) at the top of the ISR before any other logic. Not RMW - a read-modify-write on a write-1-to-clear register risks clearing flags that fired between the read and the write-back.

**Root cause at the register level:**
-

# 2026-08-15

**Morning:**
- Completed the JOURNAL logs for 12.08, 13.08 and 14.08.
- Wrote the README.md for BMP280 + I2C project.
- Wrote the README.md for the Integration Project.

**Evening:**
- Learned (reminded myself) basic informations about DMA. Started designing DMA in the UART driver to remove the CPU from serial transmission entirely.

**Design decisions:**
- DMA1 Stream 6 Channel 4 is the only valid hardware path for USART2_TX. The DMA controller's CHSEL arbitration logic only forwards peripheral requests to the stream when the channel number matches when the peripheral is actually driving. Selecting an arbitrary stream/channel combination means the DMA engine never receives a TXE request from USART2 and the transfer never starts - no error, just silence.

- S6PAR and S6M0AR:
1. S6PAR holds the fixed peripheral destination address - USART2->DR. Without S6PAR set, the stream has no target address and silently misdirects every write.
2. S6M0AR holds the source buffer address.
3. S6NDTR holds the transfer count. Set to 64. Hardware decrements this on each completed item and makes it read-only when EN = 1.

- Write order constraint:
1. CHSEL, DIR, MSIZE, PSIZE, MINC, PINC and related configuration fields are protected and writable only when EN=0. Once EN=1, the stream is a live AHB bus master arbitrating for bus access. Allowing CHSEL or direction to change mid-arbitration would leave the hardware in an undefined state - the DMA engine would not know which peripheral to service or which direction to move data.
Therefore the mandatory write order is: all S6CR configuration fields -> S6PAR -> S6M0AR -> S6NDTR -> TCIE -> NVIC enable -> EN=1 last.
PAR and M0AR have no mandatory ordering between themselves - the manual only requires both be set before EN=1.

- TCIE before EN:
1. TCIE is not hardware-locked by EN. It is technically writable before or after EN.
However, setting TCIE after EN = 1 creates a real missed-interrupt window. Between the EN=1 write and the subsequent TCIE write, the CPU executes at least one instruction.

- NVIC enable before EN:
NVIC enable must precede EN = 1 for the same reason as TCIE: the TC condition must not be able to assert before the CPU is listeting for it.

- Dead code:
1. usart2_init() contained a while (!(USART2->SR & (1<<6))) loop described as waiting for an idle frame. USART_SR reset value is 0x00C0 - TC (bit 6) is set at reset. This loop never blocks under any condition.

- TCIF6 vs SR TC:
1. DMA TCIF6 fires when the DMA engine finishes pushing the last byte into USART2->DR. This does not mean the last byte has physically left the wire - it may still be sitting in DR waiting to load into the shift register, or in the shift register actively clocking out.
2. USART SR TC (bit 6) fires when the shift register has fully transmitted the last bit and no new data is pending. This is the true "wire is silent" signal.
4. Conflating TCIF6 with SR TC would corrupt any downstream operation that requires bus silence - clock gating, low-power entry, half-duplex bus release.
3. Current week's Done bar has no such downstream consumer. SR TC path requires either a polling loop (defeats non-blocking architecture) or a separate USART TC interrupt path (out of scope for this week). So the decision: TCIF6 alone is sufficient for the done bar. SR TC deferred explicitly.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-14

**Morning:**
- Finished constructing the breadboard circuit.
- Tested the circuit with the multimeter.
- Tested the flashed program. Debugged it, fixed several bugs. Now it works.

**Evening:**
- Disintegrated the breadboard circuit.
- Wrote a Linkedin post about the debug process of the Integrated Project.
- Completed the JOURNAL log for 10.08.

**Final verification:**
- Several minutes continuous operation. Both candump and UART agreeing throughout.
- Values stable and physically plausible: ~26.27-26.29°C, ~1019.9-1020.1 hPa. Realistic sensor noise floor, not drift or corruption.
- Thermal stimulus test: thumb warmed the BMP280 chip, readings climbed smoothly in real time. Sensor responding to real physical stimulus, not producing static numbers.
- Zero occurrences of "The bus is off!" events. Zero "The MCP2515 experiences unhandled error condition!" events. Clean throughout.
- Two independent verification paths confirmed agreement: UART printf and Python/python-can decode.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-13

**Morning:**
- Finished integrating BMP280 driver to the CAN Integration Project.
- Almost finished constructing the breadboard circuit.

**Evening:**
-

**What was done - TXnIF handling:**
- Added explicit bit_modify clears for TX0IF/TX1IF in drain loop handler.
- Removed (1 << 2) | (1 << 3) mask from drain loop exit condition - mask-out-of-exit-condition trick now reserved exclusively for ERRIF/TXBO (flags that genuinely cannot be cleared). TX0IF/TX1IF receive real clears now, so no special-casing needed in exit condition.
- Exit condition: while (can_intf_val & ~((1 << 7) | (1 << 5))) - clean and honest.
- Added explicit parentheses to TX flag OR condition for MISRA-adjacent readability: (can_intf_val & MCP_CANINTF_TX0IF) || (can_intf_val & MCP_CANINTF_TX1IF).
- Mask math verified: MCP_CANINTF_TX0IF = (1 << 2) = 0000 0100, MCP_CANINTF_TX1IF = (1 << 3) = 0000 1000, ORed = 0000 1100 = 0x0C.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-12

**Morning:**
- Tested and debugged the error handling mechanism of the Integration project (TXBO, warnings etc). Fixed the bugs, now it works.

**Evening:**
- Started integrating the BMP280 and its state machine to the project.

**Fault injection test - TXBO/bus-off:**
**How fault was forced:**
- Deliberate CNF bit-timing mismatch between MCP2515 and CANable - two nodes disagreeing on bit boundaries causing sustained CAN error frames -> TEC climbing -> TXBO.
- Temporary UART debug prints added at can_bus_off and grace_active transition points to observe state machine in real time. Retained as permanent one-line logging after test confirmed.

**Observed sequence - verified against defined pass conditions:**
- TXBO confirmed set via EFLG read.
- can_bus_off = 1 within one 500ms tick.
- TX halted - no frames on candump during bus-off window.
- Fault removed (bit-timing restored).
- can_bus_off cleared within one 500ms tick + 10ms grace window.
- TX resumed on candump.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-11

**Morning:**
- Finished designing the error handling for the Integration project.
- Started implementing new TXBO branch in mcp2515_canintf_handler as well as new main() function. 

**Evening:**
- Finished implementing TXBO branch in mcp2515_canintf_handler.
- Finished the main() CAN errors branch.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-10

**Morning:**
- Started developing the Integration Project: CAN Sensor Node.
- Designing the bus-off handling.

**Evening:**
- Continued development of the Integration Project and the bus-off handling.

**What was done - ERRIF branch architecture:**
**Design decisions reasoned through before writing code:**
1. Stale vs. fresh EFLG read:
- Between first mcp2515_read(EFLG) and the final "anything unresolved?" check, real SPI transactions have executed (RX1OVR/RX0OVR bit_modify calls). EFLG is a live hardware-mirrored register - error conditions can change during that window independently of code.
- If a new EFLG big sets after the original read but before ERRIF is cleared, clearing ERRIF would produce a surprise: CANINTF still set, INT still asserted low, handler re-entering forever.
- Decision: re-read EFLG fresh before the final decision. Call it eflg_val_check. Stale data driving a stuck/not-stuck determination silently drifts from hardware reality.

2. can_bus_off vs. can_intf_stuck - mutual exclusivity:
- TXBO and warning bits (TXWAR, RXWAR, EWARN) are independent hardware bits reflecting independent counter thresholds. Both can be true simultaneously.
- if (TXB0) {...} else if (other bits) {...} would silently miss warning bits present alongside bus-off.
- Decision: check TXBO independently. Check "anything else present" independently on fresh read. Both flags can fire simultaneously - they represent different questions: "is bus-off specifically active" vs. "is there an unresolved condition outside implemented scope".
- can_intf_stuck mask excludes RX1OVR, RX0OVR, and TXBO - keeping it cleanly partitioned to genuinely untracked bits only.

3. Final ERRIF branch shape:
- Read eflg_val - handle RX1OVR, handle RX0OVR (bit_modify clear each).
- Re-read EFLG fresh -> eflg_val_check.
- If eflg_val_check & MCP_EFLG_TXBO -> can_bus_off = 1. Do not clear ERRIF.
- If eflg_val_check masked to exclude RX1OVR/RX0OVR/TXBO is nonzero -> can_int_stuck = 1. Do not clear ERRIF.
- If eflg_val_check masked to exclude RX1OVR/RX0OVR is zero -> clear ERRIF.

4. Drain loop fix:
- Exit condition changed from while (can_intf_val != 0) TO while (can_intf_val & ~(1 << 5)) - ERRIF (bit 5) excluded from loop continuation condition. Prevents drain loop from spinning forever when ERRIF is unresolvable (TXBO active, can_intf_stuck set).

5. 500ms tick block - CAN fault-handling state machine:
- Unconditional tick reset on every 500ms entry.
- can_intf_stuck logged via UART on each tick it remains set - visibility without recovery logic outside current scope.
- can_bus_off state machine: non-blocking, polled at 500ms tick. Re-reads EFLG to check TXBO recovery (128x11 bit-times passive recovery). Transitions through grace_active window (10ms) before resuming TX.
- TX send path gated on can_bus_off == 0. No frames dispatched during bus-off or grace window.

6. Volatile audit:
- can_bus_off, can_intf_stuck - written in mcp2515_canintf_handler called from main loop (not ISR). Read in 500ms tick block, also main loop. No cross-context boundary. volatile not required.
- can_int_flag - written by ISR, read by main loop. volatile required. Unchanged.

**Test plan defined before touching hardware:**
- Must force TXBO deliberately - bench topology (MCP2515 + CANable with candump)ACKs frames at protocol level. TEC never climbs naturally on a correctly wired bus. Must break something: disconnect CANable, remove termination, or mismatch CNF bit-timing. 
- UART debug prints added temporarily in can_bus_off / grace_active transition points to observe state machine in real time.
- Pass condition defined precisely: TXBO confirmed via EFLG read -> can_bus_off = 1 within one 500 ms tick -> no frames on candump during bus-off window -> fault removed -> can_bus_off clears within one 500ms tick + 10ms grace -> TX resumes on candump.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-09

**Morning:**
- Designed how to walk around the 4.7V issue since the circuit and all its components are ok. Connected the INT line from the MCP2515 to the level-shifter and connected it with the PB15 on the STM32.
- Tested the Interrupt program. Works wonderfully.
- Implemented two-stage length-aware buffer read, with header and payload properly separated and returned to the caller, because constant 13-byte read is not always optimal solution (especially when data payload is 1-2 out of 8 possible bytes). 
- Tested the Interrupt program with new buffer read mechanism. Works as expected.
- Completed the Journal logs from 01.08 to 09.08.

**Afternoon:**
- Wrote the MCP2515 README.md.

**Evening:**
- Wrote the test harness STM32 + TXS0108E + MCP2515 + CANable README.md.

**What was done - INT pin wiring resolution:**
- TXS0108E internal pull-ups confirmed from datasheet: "pull-up resistors have been conveniently integrated so that an external resistor is not needed". Both A-port and B-port have internal Rpua/Rpub to VCCA/VCCB respectively.
- Routed INT through unused TXS0108E HV channel. PB15 into matching LV channel. OE already tied high from original setup - internal pull-ups active.
- External 2.2 kOhms resistor taken out from the breadboard circuit since it is redundant.
- Static idle voltage on PB15 side: clean 3.3V confirmed.
- Note on TXS0108E pull-up mechanism: dynamic, not fixed. 4 kOhms when driving high (fast rising edge - one-shot accelerator), 40 kOhms when settled high (power saving).

**Length-aware RX buffer read:**
- Refactored mcp2515_read_rx_buffer() to two-stage read: header buffer [5] (SIDH, SIDL, EID8, EID0, DLC) and payload buffer [8] (data bytes, DLC-length only).
- Caller receives IDE and DLC information explicitly rather than it being discarded inside a fixed 13-byte read.

**Volatile analysis - can_int_rx0_header / can_int_rx0_payload / can_int_rx0_flag:**
- ISR sets can_int_flag -> main() drain-loop calls mcp2515_canintf_handler() -> mcp2515_read_rx_buffer() -> spi_transfer().
- can_int_rx0_header / can_int_rx0_payload written and read exclusively within main()'s sequential execution. ISR never touches them directly.
- volatile is not required on these variables. No producer/consumer relationship across interrupt boundaries. Compiler has no incentive to cache or reorder them incorrectly.
- can_int_flag correctly stays volatile - that is the actual cross-context handoff between ISR and main().

**Hardware verification - DLC = 2 test frame:**
- Sent: cansend can0 1F2#AABB
- can_int_rx0_header: {0x3E, 0x40, 0xDE, 0xA4, 0x02} - SIDH/SIDL correct for iD 0x1F2, EID bytes garbage (expected, standard frame), DLC = 2 correct.
- can_int_rx0_payload: {0xAA, 0xBB, 0x9C, 0x9D, ...} - first two bytes exact match. Bytes 2-7 stale from previous 8-byte test, unwritten this frame. Correctly scoped by DLC - any caller reading DLC & 0x0F bytes gets correct data, anything beyond is uninitialized-for-this-frame.

**Status: MCP2515 Interrupt-driven CAN driver - DONE.**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-08

**Morning:**
- Learned about why oldest frame survives during RX buffer overrun in CAN and the newest one discards.
- Learned about Active-Low/Active-High and Rising Edge/Falling Edge triggers.
- Implemented CAN ISR (EXTI15_15_IRQHandler) and mcp2515_canintf_handler that handles the dispatch mechanism.
- Adjusted main() for ISR.

**Afternoon and evening:**
- Added the INT line to the breadboard circuit.
- Noticed that INT line has 4.7V, instead of expected 3.3V pull-up idle state. Debugged it.

**Problems encountered:**
**Bug:** INT line reading 4.7V instead of expected 3.3V pull-up idle state
**Symptom:** Resistor leg A (connected to 3.3V rail) reads 3.3V correctly. Resistor leg B (shared row with PB15/INT wiring) reads 4.7V. STM32 PB15 pin isolated, nothing connected - reads 120mV floating (normal). No code flashed touching PB15. Pure electrical issue.

**Diagnosis process:**
- A passive resistor cannot generate voltage higher than what's on either leg. 4.7V on Leg B with 3.3V on Leg A -> something else actively feeding that row from a different wire.
- Ruled out STM32 pin - floats to 120mV when isolated, not the source.
- Rules out level shifter - INT was never wired to it.
- Remaining suspect: MCP2515 module INT pin itself.
- Test: physically unplugged wire from MCP2515 INT pin to breadboard row. Multimeter immediately dropped to 3.3V -> confirmed MCP2515 INT wire was the source.

**Further investigation - onboard pull-up hunt:**
- Suspected onboard pull-up resistor on INT to module's 5V VDD rail (common on Arduino-target breakout boards).
- Measured INT-to-VCC and INT-to-GND resistance directly at module header: both returned ~3.5 MOhms - identical readings to both rails.
- Conclusion: not a real discrete resistor. Multimeter test current finding nonlinear leakage path through chip's internal ESD clamp diodes/protection circuitry. A genuine 10 kOhms pull-up would read ~10 kOhms to VCC and opend to GND - identical high readings rule it out entirely.
- R3 resistor visible between VCC and INT is serving a different net entirely. Proximity on PCB was misleading.
- INT is a true open-drain, no onboard pull-up, high-impedance floading node when idle - consistent with datasheet description.

**Root cause of original 4.7V reading:**
- Floating high-Z node picking up capacitive/inductive coupling from adjacent 5V VCC trace on densely packed module PCB. Multimeter's own input loading made it appear artificially stable. Measurement artifact, not a driven voltage. Same class of artifact as the 120mV floating PB15 reading, different magnitude.

**Resolution:**
- After extensive investigation confirming no genuine electrical fault exists on the module, routing INT through the TXS0108E level shifter. Already proven infrastructure for CS/SCK/MOSI. Reuses validated path rather than debugging a new direct-wire path from scratch.
- Note: original plan was INT direct to PB15 (5V-tolerant FT pin, no shifter needed, same reasoning as MISO). Pragmatic decision to use shifter given time spent chasing floating-node artifact. Electrically either path is valid.

**Root cause at the register level:**
-

# 2026-08-07

**Morning:**
- Continued developing the CAN interrupts ISR.

**Evening:**
- Finished developing the CAN interrupts RX and ER. 
- Started implementing the CAN interrupts RX and ER: added SYSCFG and EXTI register maps, implemented mcp2515_init() for INT line configuration.
- Wrote a Linkedin post about RX0OVR and "oldest-frame-wins. newest-frame-drops".

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-06

**Morning:**
- Learned about Acceptance Filters and Acceptance Masks in CAN RX.
- Learned about MAB (Message Assembly Buffer) in CAN RX.
- Started learning CAN Interrupts: the theory is done, the design/development and implementation are next.

**Evening:**
- Learned about EXTI.
- Learned about SYSCFG.
- Started designing the CAN Interrupt ISR.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-05

**Morning:**
- Implemented the RX path test harness for the CAN bus of STM32 + MCP2515 and the CANable. 
- Tested the RX path test harness. Works as intended.
- Designed and derived the EFLG error handling (RX0OVR) - deliberately forcing an error condition to prove the driver detects it.

**Evening:**
- Tested the EFLG error handling test harness.
- Derived the mechanism of OVR: the unread data frame is not overwritten with the newest one. If the RXB0 is full, the BUKT is not set, so the roll over is not allowed, the OVR bit sets and the newest data frame in the MAB (Message Assembly Buffer) is discarded. If the roll over is allowed to the RXB1, but it is also full, the data frame is discarded as well.

**What was done:**
- Sent frame from PC via cansend: ID = 0x1F2, payload = DEADBEEF (4 bytes).
- STM32 polled RX0IF (bit 0 of READ STATUS response) until frame arrived.
- Read RX buffer starting at MCP_Read_RXB0SIDH.
- Result:

```c
rx_frame_bytes = {0x3E, 0x40, 0xDC, 0x24, 0x04, 0xDE, 0xAD, 0xBE, 0xEF, ...}
```

**Verification byte-by-byte:**
- SIDH = 0x3E, SIDL = 0x40 -> decodes back to ID 0x1F2. Correct - matches cansend ID, frame not generated by STM32 itself.
- EID8/EID0 = 0xDC/0x24 - undefined bits for standard-ID frame. Same don't-care garbage reasoning as loopback test. Not a bug.
- DLC = 0x04 - correct. DEADBEEF = 4 bytes, cansend length-encodes automatically from hex string length.
- DE AD BE EF - exact payload match.

**Status: MCP2515 polling driver - DONE by project bar.**
Bidirectional frame exchange confirmed with independent node. Hand-derived bit timing. Hand-verified register sequences. No vendor HAL, no CAN library anywhete in the stack.

Happy path works. Defined error path handled (mode-switch timeout + LED). No races. No bus hangs. Bounded polls where it matters.

**What was done - EFLG RX0OVR:**
**Mechanism reasoned through before touching code:**
- RX0OVR requires two simultaneous conditions: valid message arrives for RXB0, AND RX0IF already set (buffer not yet read by MCU).
- BUKT bit in RXB0CTRL must be 0 (rollover disabled). If BUKT = 1, second frame rolls into RXB1 instead of triggering overflow. Default after reset: BUKT = 0 - no explicit write needed.
- Test method: poll RX0IF until first frame arrives, then deliberately skip mcp2515_read_rx_buffer. Hold there. Fire second cansend from Linux. Second frame lands on RXB0 while RX0IF still set -> overflow triggered.

**Hardware result:**
- EFLG read: 0b10000000 = bit 6 set = RX0OVR confirmed.
- rx_frame_bytes after reading RXB0: {0x3E, 0x40, 0xDE, 0xA4, 0x04, 0xDE, 0xAD, 0xBE, 0xEF} - first frame's payload DEADBEEF, not second frame's FEFDFEEF.

**Datasheet flowchart confirmed on hardware:**
- Overflow check happens before any write into RXB0. When second message finishes assembling in Message Assembly Buffer and finds RX0IF = 1 with BUKT = 0, MAB contents are discarded. RXB0 never touched - holds first frame untouched.
- Behavior: oldest-frame-wins, newest-frame-dropped. RXB0 contents remain valid, just old. Not silent corruption - the data is still the original frame, just stale.

**Status: EFLG RX0OVR - DONE.**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-08-04

**Morning:**
- Developed the Normal Operation mode change from the Loopback in the test harness.
- Derived the real CAN bus transaction between the STM32 + MCP2515 and the CANable node connected to my PC.
- Implemented the test harness including the error handling if the mode has not been set during 10 ms window before the timeout.

**Evening:**
- Tested the network of the STM + MCP2515 and the CANable node. Works perfectly. `can0  1F2   [5]  3F F1 B4 11 AA`
- Started implementing receiving path from the CANable to the STM32 + MCP2515: filters and masks.

**What was done - Normal mode transition:**
- mcp2515_bit_modify(CANCTRL1, 0xE0, 0x00) - mask covers bits 7:5 (REQOP), data = 0x00 (Normal mode). Leaves rest of CANCTRL undisturbed.
- Polled CANSTAT bits 7:5 (OPMOD) for 000 to confirm actual mode transition - write to CANCTRL, verify in CANSTAT. Same "write one register, verify in another" discipline as RESET->CANSTAT verification.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
- Loopback mode never touches CANH/CANL - TX buffer route internally to RX buffers. CANable sees nothing in Loopback. Normal mode is the only mode where two independent nodes can communicate on a real bus.
- Two nodes on a CAN bus must agree on bit rate almost exactly. CNF registers set the MCP2515's timing. CANable's SocketCAN driver sets its own timing independently via bitrate 500000. Mismatch would cause bit-stuffing errors and garbled frames - not silence.
- kbit/s = kbps numerically, but SocketCAN takes raw bit/s integer: 500000, not 500 or 500k.

# 2026-08-03

**Morning:**
- Implemented the Loopback test harness: data frame construction, TX buffer loading, status polling, RX buffer reading, bytes comparison.
- Tested on the breadboard circuit, verified the results, fixed bugs.

**Evening:**
- Added the CANable analyzer adapter to the breadboard circuit: connected and tested.
- Designed and developed the Normal Operation mode test harness to test two-node CAN communication.

**Problems encountered:**
1. Bug 1: Polling loop executing exactly once regardless of RX status
**Root cause:**
- while (status_val & 0) - status_val & 0 evaluates to 0 for any value of status_val. Loop condition always false. Loop body executed exactly once then exited, regardless of whether RX0IF was actually set.

**Fix:** while (!(status_val & (1 << 0))) - isolates bit 0 (RX0IF), inverts it. Loop cycles while bit 0 is 0 (no interrupt pending), exits when RX0IF goes high.

2. Bug 2: RTS call passing enum value by value instead of by pointer
**Root cause:**
- mcp2515_rts(location, 1U) - function signature expects MCP_RTS_locations_t \*locations. Passed the enum value directly instead of its address.

**Fix:** mcp2515_rts(&location, 1U)

3. Bug 3: Byte-for-byte comparison across RX/TX buffer layouts - reserved bits mismatch
**Root cause:**
- RXBnDLC bits 5:4 (RB1/RB0) are "Reserved" - not "Unimplemented: Read as 0". No documented guarantee about their read-back value. TX side had those bits at 0 by construction. Naive byte-for-byte comparison could produce false mismatch on bits that have nothing to do with test success.
- RXBnSIDL bits 1:0 (EID[17:16]) also carry no guaranteed read-back value for standard-ID frames - verified empirically: consistent 0xDE, 0xA4 across all runs, deterministic but meaningless silicon-level state the CAN engine never touches.

**Fix:** Masked DLC comparison to only meaningful bits - bit 6 (RTR) and bits 3:0 (DLC count). Bits 7/5/4 excluded. DLC byte special-cased inside the comparison loop with explicit mask rather than uniform byte comparison.

**Final loopback result:**

```c 
rx_frame_bytes = {0x3E, 0x40, 0xDE, 0xA4, 0x05, 0x3F, 0xF1, 0xB4, 0x11, 0xAA}
```

- SIDH = 0x3E, SIDL = 0x40 - exact match for hand-derived ID 0x1F2 packing.
- DLC = 0x05, data bytes 0x3F/0xF1/0xB4/0x11/0xAA - exact round-trip match.

**Status: MCP2515 polling driver - DONE by project bar**
Reset, mode-switching via BIT_MODIFY, multi-byte auto-incrementing writes, frame construction, transmit request, status polling, and buffer retrieval with automatic flag-clear - all proven on real silicon.

**Next steps:**
- Exit Loopback mode into Normal mode. Test against CANable analyzer on real physical bus.
- Interrupt-driven path (CANINTE/INT pin) deferred to later phase.

**Empirical verification-  EID reserved bytes:**
- Ran multiple loopback iterations. rx_frame_bytes[2] and rx_frame_bytes[3] (EID8/EID0 positions) returned 0xDE and 0xA4 consistently acrosss every run.
- Consistent across runs = not noise, not leftover data, not anything the CAN engine touches for standard-ID frames. Most likely fixed silicon-level SRAM power-on state - the internal RX buffer bytes that correspond to extended ID fields simply never get written during a standard-ID frame reception.
- Theory confirmed empirically: don't accept "plausible explanation" for undefined behavior. Test it. Same value every run is a result. Random values every run is a different problem entirely.

**Root cause at the register level:**
-

# 2026-08-02

**Morning:**
- Built and tested with multimeter the breadboard circuit of the STM32F411, TXS0108E and MCP2515.

**Afternoon:**
- Implemented main.c test bench: reset -> configure bit timing -> switch operating mode. Each step independently verified by readback.

**Evening:**
- Derived loopback test plan: build a frame, load_tx_buffer, rts, then poll and retrieve: read_status loop with mask bit 0 (RX0IF), retrieving of the frame and clearing RX0IF, received byte comparison against what has been sent.
- Wrote a Linkedin post about the SPI + EEPROM project.
- Completed the 1st August Journal entry.

**What was done:**
- Built breadboard circuit: STM32F411 Black Pill + TXS0108E level shifter + MCP2515 CAN controller module.
- Rail split: right rails = 3.3V, left rails = 5V. TXS0108E straddles the separation row - VA (3.3V) side faces right rails, VB (5V) side faces left rails.
- Level shifter placed directly behind Black Pill to keep A-side hop short per capacitance budget.
- MCP2515 module sits off-board (male pins, not breadboard-mounted).

**First hardware bring-up test:**
- Sequence: SysTick_Init -> spi_init(SPI_BR_8) -> mcp2515_reset() [includes 2ms OST delay] -> mcp2515_read(0x0E, &rx_byte, 1U) -> breakpoint.
- Result: rx_byte = 0x80 = 10000000 binary. Exact predicted value.

**What 0x80 proves end-to-end:**
- GPIO/AF/MODER configuration for SPI1 correct.
- BR divider producing working clock.
- TXS0108E correctly translating SCK/MOSI/CS from 3.3V to 5V - if it weren't, MCP2515 would have never received a coherent RESET instruction.
- MISO direct FT-pin path correctly carrying 5V-driven data back to STM32.
- OST delay sufficient - too short would have caught device mid-reset and returned garbage.
- Instruction framing (opcode + address byte, correct tx_len/rx_skip) byte-accurate.
- MCP2515 genuinely landed in Configuration mode after reset as datasheet promised.

**Next step:**
- Write CNF1/CNF2/CNF3 (0x00/0x91/0x01) while in Configuration mode.
- Read back to confirm writes landed.

**Problems encountered:**

1. Bug 1: OR-ing mode bits without clearing existing REQOP field first
**Root cause:**
- CANCTRL_val |= MCP_Loopback_Mode - REQOP bits 7:5 already held 100 (Configuration mode). OR never clears bits, only sets them. 100 | 010 = 110 - neither Configuration nor Loopback. Wrong value written.
- Using the target value's bit pattern as if it were a bitmask - two different things. A mask says "which bits am I allowed to touch". A value signals "what do I write into them".

**Fix:** Clear bits 7:5 first with CANCTRL_val &= ~(0x7 << 5), then OR in the new mode value. Or use mcp2515_bit_modify() which separates mask and data by design.

2. Bug 2: Passing pre-read register snapshot as data argument to BIT_MODIFY
**Root cause:**
- mcp2515_bit_modify(CANCTRL1, mask, CANCTRL_val) - CANCTRL_val still held the old Configuration mode value. BIT_MODIFY writes the data argument's bits wherever mask = 1. Passing the old value told hardware "set these bits to what they already are". Nothing changed.

**Fix:** PASS MCP_Loopback_Mode directly as the data argument - already correctly positioned at bits 7:5 from its << 5 definition.

3. Bug 3: Incomplete mask missing bit 5 of REQOP field
**Root cause:**
- mask = (1 << 7) | MCP_Loopback_Mode. MCP_Loopback_Mode = (0x2 << 5) = 0100 0000 - only bit 6 set. Combined mask = 1100 0000 - bits 7 and 6 only. Bit 5 of REQOP never covered. Used the target value's bit pattern as the mask instead of deriving a proper 3-bit-wide field mask.

**Fix:** mask = (0x7 << 5) = 1110 0000 - covers all three REQOP bits regardless of what mode is being written.

**CAN frame construction - verified:**
- ID packing: SIDH = id >> 3, SIDL = (id & 0x7) << 5.
- DLC byte: RTR = 0 (data frame) DLC[3:0] = plain 4-bit byte count. 0000 0110 for 6 bytes.
- LOAD TX BUFFER streams continuously - same open-ended convention as WRITE instruction. ONe single mcp2515_load_tx_buffer() call, streaming full frame: {SIDH, SIDL, EID8, EID0, DLA, D0...Dn}.

**Root cause at the register level:**
-

# 2026-08-01

**Morning:**
- Revisited SPI CAN instruction codes.
- Implemented SPI instruction layer - thin wrappers around spi_transfer().
- Parameterized SPI init with customized BR value + enum with 7 available bit sequences for the BR.

**What was done:**
* Derived CAN bit-timing configuration for 500 kbit/s at F_OSC=8MHz.
* Final register values: CNF1=0x00, CNF2=0x91, CNF3=0X01. Sample point at 75%.
* Verified all constraints: PropSeg+PS1 >= PS2, PS2 > SJW, valid field ranges, correct encoding conventions checked individually.
* Identified CNF3 bit 7 (SOF) as tied to CLKEN/CANCTRL - don't-care/0 by default, not something to set manually.

**Electrical/physical layer - derived and documented:**
* TXS0108E level shifting on SCK/MOSI/CS. MISO direct via FT pin. OE tied to VA. Decoupling caps identified.
* Capacitance budget re-derived for actual shifter topology: 23pF, comfortably under 50pF ceiling.
* SPI clock budget: 2MHz, justified against real t_PD. BR divider correctly computed from APB2 clock (16MHz/8).

**SPI instruction layer (mcp2515.c/.h) - written, unverified on hardware:**
* All nine MCP2515 instructions framed correctly with proper tx_len/rx_skip patterns.
* const-correctness fixed.
* Self-documenting enums matching real datasheet semantics: RTS as bitmask, LOAD_TX as named buffer+offset pairs.
* Prototypes properly declared.

**Driver skeleton build order established:**
* SPI instruction layer - RESET, READ, WRITE, BIT MODIFY, READ STATUS, RTS opcodes. Thin wrappers around spi_transfer().
* Mode control - request Configuration mode via CANCTRL REQOP bits, poll CANSTAT until confirmed. Same request/poll pattern as bxCAN.
* Bit timing init - write CNF1/CNF2/CNF3 only while in Configuration mode.
* Return to Normal/Listen-only/Loopback mode - same request/poll pattern out of Configuration mode.

**Status: unverified**
Will validate against real silicon. CANable analyzer as independent verification instrument. "Done" not claimed until happy path confirmed on hardware.

**Afternoon:**
- Soldered 3 ordered TXS0108E with their header pins.

**Evening:**
- Started building breadboard circuit from Blackpill, TXS0108E level shifter and MCP2515.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
- Deriving register values from first principles before building a breadboard circuit is legimate prep work. Claiming "done" before hardware confirms it is not.
- SPI clock rate should not be a guess - it is a budget derived from level shifter propagation delay, bus capacitance, and MCU APB2 clock. Every number needs a traceable reason.
- Parameterize SPI init function for clock rate so future devices on the same peripheral can pick their own rate without duplicating the init function.

# 2026-07-31

**Morning:**
- Calculated the CAN bit timing, the CNF1/CNF2/CNF3 derivation and their final hex values.
- Started implmenting MCP2515 driver: instruction codes.

**Evening:**
- Learned MCP2515 datasheet topics: Bit timing, Modes of Operation, SPI Interface. Enough of theory. Time to immerse myself into practice :) 

**Problems encountered:**
- MCP2515 instruction set: base opcodes + selector enums (RX buffer offset-encoded, TX buffer flat-named per table, RTS as bitmask) - caught two bugs: unassigned enum values defaulting to sequential int, and non-self-documening binary-pattern names.
- I definitely need to reread and fix my mental model of the intructions today evening :)

**Root cause at the register level:**
-

# 2026-07-30

**Morning:**
- Ordered MCP2515 and the level shifter.
- Learned about the level shifter and what they do.
- Remind myself the derivation of connectivity between STM32 3.3V and MCP2515 5V.
- Continued to learn about CAN bus.

**Evening:**
- Started learning about CAN bit timing and calculating it. 

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-07-29

**Morning:**
- Started learning CAN bus.
- Realized the STM32F411 does not support CAN, so try to figure out how to get around it.

**Evening:**
- Derived the electrical/timing side of Level shifter + MCP2515 + STM32 SPI. Need to order the lever shifter and MCP.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-


# 2026-07-28

**Morning:**
- Learned and wrote 250-300 English words that have been collected from reading manuals and other sources.

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-07-27

**Morning:**
- Wrote an SPI driver README.

**Evening:**
- Finished a M95320 driver README.
- Finished the test harness for this project README.
- Wrote a LinkedIn post about verification of the fault paths.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-07-26

**Morning:**
- Soldered 4 M95320WT EEPROM with SOP-to-DIP adapters.
- Built an electrical circuit on the breadboard of the Blackpill, the EEPROM and two LEDs. Tested it.

**Afternoon:**
- Tested the circuit with a multimeter.
- Flashed and tested the program on bare-metal. It works.
- Tested the fault branch as well. Works.
- Enhanced the learning plan.

**Evening:**
- Wrote the descriptions of bugs and interesting moments happened during the SPI + M95320 drivers development.

**Test harness:**
- Not particularly a problem, but found out that the test harness in main loop did not have delays after the led_off, what made the LED immediately go back to the ON state. No visible toggle. After adding some delay after the led_off, there are distinguishable ON and OFF LED states.
- Tested the fault branch of the test harness by placing data_arr[0] = 0xF1 deliberatly before the first comparison between the read byte and the byte that was "written" to the EEPROM. As expected, the program went to the fault mode, the green LED silent completely, the red LED blinking infinitely.

**Root cause at the register level:**
- 

**Lesson learned:**
- You need to verify the test harness before and during its execution.
- Break the function/test harness deliberately to see if the parts of its are reachable and executing correctly.

# 2026-07-25

**Morning:**
- Enhanced EEPROM functions, implemented eeprom_write_byte function.
- Rewrote spi_transfer function.
- The M9520 EEPROM driver v.1.0 is done. main() is next.

**Evening:**
- Implemented the main(). Will solder the M9520 EEPROM tomorrow. Will test it, debug and complete in the Journal all bugs, fixes and interesting parts of the project.

**Problems encountered:**
1. Bug: WRITE_INSTRUCTION opcode collision with WRDI
**Symptom:** Silent write failure - data never changes, no error raised. WIP bit never goes high after eeprom_write_byte() call.

**Root cause:**
- WRITE_INSTRUCTION defined as 0x4 - same value as WRDI_INSTRUCTION. Correct WRITE opcode is 0x2.
- Every eeprom_write_byte() call transmitted Write Disable opcode instead of Write to Memory Array. EEPROM decoded it, reset WEL, entered deselect wait state. No write cycle triggered.
- WIP never goes high because WIP only asserts during WRITE or WRSR instructions - WRDI does not trigger it.

**Fix:** #define WRITE_INSTRUCTION 0x2

2. Bug: eeprom_read() address bytes pulled from uninitialized output buffer

**Root cause:**
- address parameter never referenced in function body. tx[1] and tx[2] populated from result - the output buffer - which contains uninitialized stack garbage at that point.
- EEPROM received two random stack bytes as  memory address. Per Table 4, those bits directly select which of the 4096 byte locations gets read. Random address read, not intended one.

**Fix:** tx[1] = address[0] (MSB - top nibble unused, lower nibble), tx[2] = address[1] (LSB - A7:A0).

**Lesson learned:**
- Always acknowledge compiler unused-parameter warnings.

3. Bug: eeprom_read() - response buffer offset corrupting read data

**Root cause:**
- spi_transfer(tx, result, len) called with len = total transfer length (instruction + 2 address bytes + N data bytes.
- result[0], result[1], result[2] overwritten with garbage bytes from MISO captured during instruction and address phase. MISO is high-impedance until address is fully clocked in.
- Real EEPROM data starts at byte 4 onward - lands at result[3..N+2], not result[0..N-1] as caller expects.

**Fix:** Restructure eeprom_read() as well as spi_transfer() to support partial array handling, introducing the variables like rx_skip to handle how many elements to skip before writing to the RX array etc.

4. Bug: Comma operator instead of && in loop condition.

```c
for (uint8_t i = 3, i_payload = 0; i < transfer_length, i_payload < len; i++, i_payload++)
```

**Root cause:**
- I believed that it is a perfectly compilerable piece of code since all three clauses of the for loop use commas because there are two tracking variables.
- I found out that the loop continuation is controlled only by the second part of it: i_payload < len. i < transfer_length is silently discarded.

**Fix:**
- Change the comma to the && operator: i < transfer_length && i_payload < len. In this case, both conditions are evaluated and the loop continues only if both condition parts are true.

**Lesson learned:**
- Silent failure (no error, no WIP, no crash) is the hardest class of bug.
- Always check compiler warnings.
- Never assume that comma and && are interchangable :)

# 2026-07-24

**Morning:**
- Read the EEPROM M95320 datasheet Instruction section to grasp what it allows to do before writing the EEPROM driver.

**Evening:**
- Started implementing M9520 EEPROM driver: eeprom_read, eeprom_write_enable and eeprom_wip_poll functions first versions.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-23

**Morning:**
- Developed and wrote spi_transfer function.
- Wrote a Linkedin post about NVIC priorities.

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-


# 2026-07-22

**Morning:**
- Wrote a `spi_init` function.
- Started to learn about GPIO OSPEEDR and breadboard's parasitic capacitance calculations.

**Evening:**
- Finished the spi_init function.
- Continued developing the SPI + EEPROM project: designed spi1_transfer function. 

**Problems encountered:**
- Definitely I need to read the Instructions section of the M95320 EEPROM to grasp it :)

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-21

**Morning:**
- Finished learning theory about SPI: CMOS with NMOS and PMOS, full-duplex vs half-duplex, SPI's transmit functionality, push-pull (SPI) vs open-drain (I2C).

**Evening:**
- Started developing the SPI + EEPROM M95320-W project: added SPI struct and addresses to the stm32f411.h, started developing the program logic.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
- SPI is fast, but is ineffective for long-distance communications.

# 2026-07-20

**Morning:**
- Started learning SPI: Serial Peripheral Interface.

**Evening:**
- Continued learning about SPI: its modes, the lines, the status register flags, its pros and cons.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-19

**Morning:**
- Added the bug descriptions to the Journal logs.
- Wrote a Linkedin post about I2C + BMP280 Interrupt-Driven project.
- Cleaned a bit my Linkedin profile.

**Afternoon:**
- Wrote a README for I2C driver.

**Evening:**
- Wrote a README for BMP280 driver.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-18

**Morning:**
- Final UART and race ISR_EV bugs found and fixed.
- Tested the interrupt-driven I2C + BMP280 program. It works.

**Afternoon:**
- Started completing the JOURNAL entries with the debug info.

**Problems encountered:**
1. Bug: Stale DR read captured as fake data[0] - repeated-start BTF reentry corrupting RX buffer.

**Symptom:** `raw_calib[0] == 0x88` (== `BMP280_REG_CALIB_START`), `raw_adc[0] == 0xF7` (= `BMP280_REG_PRESS_MSB`). Both independent burst reads: byte index 0 of received buffer exactly equals the register address byte transmitted in the write phase, not real sensor data. Systemic to all N>=3 `I2C_TX_RX` reads.

**Root cause at the register level:**
- In the TX_RX write-phase BTF branch, ITBUFEN is enabled and `hi2c.phase` is flipped to I2C_TX_RX_READ - but `hi2c.state` is never written. State remains at whatever it was holding entering the branch.
- BTF is a level flag. The repeated START condition doesn't clear BTF intantaneously - START takes real bus time to generate. ITBUFEN is already open.
- ISR re-enters immediately. Guard checks `state` - state was never transitioned to anything the guard rejects. Guard passes.
- Now inside the ISR with `phase == I2C_TX_RX_READ`, `RxLength >= 3`, `index == 0` - no tail-end index matches. Falls through to the mid-burst fallback `else` branch.
- Fallback reads DR into `pRxBuffPtr[0]`. But no real SB/ADDR/RXNE event has occurred yet - DR still physically holds the register address byte from the write phase. Fake byte captured as data[0].
- All subsequent bytes shift by one position. 24-byte calibratiion burst produces 24 bytes but starting from the wrong point - first real sensor byte lands at index 1, last real byte never captured.

**Secondary bug identified:**
- `I2C_Master_Transmit_Receive` still contains `hi2c.state = I2C_STATE_TX_ADDR` written before `CR1 |= START` - the same premature-state-write bug already fixed in `I2C_Master_Transmit` with `I2C_STATE_START_PENDING`. Fix was never propagated to this function. Classic example of fixing bug in one function without auditing structurally identical functions elsewhere.

**Fix - repeated-start reentry:**
- Transition `hi2c.state = I2C_STATE_START_PENDING` inside the write-phase->read-phase BTF branch, immediately after enabling ITBUFEN and writing the repeated START. This gives the ISR guard a state it rejects on reentry - same mechanism that protected the initial START from phantom SB handling.
- The repeated-START is electrically identical to the initial START: a request sent to hardware, waiting for genuine SB confirmation. `I2C_STATE_START_PENDING` is semantically correct for both.

**Fix - premature state write in `I2C_Master_Transmit_Receive`:**
- Replace `hi2c.state = I2C_STATE_TX_ADDR` with `hi2c.state = I2C_STATE_START_PENDING`, written before `CR1 |= START`, matching the corrected pattern in `I2C_Master_Transmit`.

**Deferred - structural note for future:**
- The repeated-start implementation in this driver has an inherent window between write-phase BTF handling and genuine SB confirmation where the ISR guard must correctly reject reentry via state rather than phase.
Since the BTF flag is cleared by hardware once START or STOP condition is detected on the bus and in I2C_TX_RX case we are not explicitly clearing the BTF, but just issuing the REPEATED START after the ISR exit, the BTF flag is still set.
Since the BTF is still set when the ISR exits, the pending interrupt flag in NVIC has not been cleared, causing immediate re-entry.
The ISR immediately re-enters itself and hits the BTF once again for the same stale BTF flag.
The EV ISR guard checks hi2c.state == I2C_STATE_START_PENDING at entry and discards all re-entries while the repeated START is pending on the bus (observed in GDB: races on approximately 9 of every 19 repeated-start cycles).
The I2C_Master_* functions that issue START have I2C_PollHardwareBusy that takes up to 4 ms of time, so there are almost no races at all.
The ISR issuing START has no time-buffer at all.
The issue is just frequent, but cosmetic - no side effects if the guard is present.

- Marked in code comments for the future examination and resolving.

**Lesson learned:**
- Fixing a bug in one function does not fix it in structurally identical functions. Always audit siblings after any structural fix.
- A phase transition without a corresponding state transition leaves the ISR guard blind. State and phase must move together at every architectural boundary - not just at transaction start and end, but at every mid-transaction pivot point that changes what the ISR is allowed to do next.
- DR holds its last written value until something reads or overwrites it. Any premature DR read - before a genuine hardware even confirms real data has arrived - captures stale contents, not sensor data. Hardware does not clear DR between write phase and read phase of a repeated-start transaction.

**Status: v3 interrupt-driven I2C driver - DONE by project bar.**
Happy path works. No bus hangs. No race conditions in current architecture. Defined error conditions handled.

# 2026-07-17

**Morning:**
- Continued debugging.

**Evening:**
- Continued debugging and chasing bugs: fixed Master_Transmit bug, found couples more. The full BMP + I2C program still does not work yet.

**Problems encountered:**
1. Bug: BTF->STOP branch firing twice per transaction - double STOP write corrupting next transaction's START.

**Symptom:** Program hangs after the first successful execution. UART prints calibration line once, then stalls. Second transaction's ADDR never arrives despite START being written.

**Root cause at the register level:**
- BTF is a level flag. It stays asserted until the physical STOP condition completes on the bus - not until CR1 STOP bit is written by firmware. Writing `CR1 |= (1 << 9)` takes zero CPU cycles; generating the actual STOP condition on SCL/SDA takes real bus time.
- `I2C_STATE_FINISHING` was not in the ISR guard's exlude list. ITEVFEN still enabled. ISR returned after writing STOP, BTF still physically asserted, NVIC immediately re-triggered the same handler.
- Second entry: `state == FINISHING`, `mode/index/TxLength` unchanged - same branch ran again, OR-ing `CR1 |= (1 << 9)` onto a STOP bit still mid-generation on the wire.
- Re-asserting STOP while the first STOP is still electrically resolving corrupyed the bus state. Next transaction's START landed on the top of an unresolved STOP condition - SB/ADDR sequence never produced a valid result.
- Fourth distinct race found during this session, all the same structural shape: guard exclude-list missing a state that the ISR can legitimately re-enter with state but matching conditions.

**Fix:**
- Added `I2C_STATE_FINISHING` to the ISR guard exclude list. Handler now returns immediately if state is `ERROR`, `IDLE`, `DONE`, or `FINISHING` - before taking any SR1 snapshot or entering any dispatch branch.

**Architectural note - exclude-list vs include-list:**
- Exclude-list requires a new entry every timea state is added or touched. Demostrated three times over in one session.
- Include-list (reject unless state is one of the few states where ISR has legitimate work: `TX_ADDR`, `RX_ADDR`, `START_PENDING`) is structurally safer - doesn't depend on remembering to update it after every state addition. Conversion deferred but noted.

**Validation:**
- Sustained clean UART output across multiple Init->ReadCalibration->Measuring cycles. No stall. No hang.

**Lesson learned:**
- Register writes that generate bus events are not instantaneous - this was already in the journal from the polling driver. The interrupt-driven driver re-learned the same lesson: STOP written to CR1 != STOP completed on wire. Any flag gated on STOP completion stays asserted until the wire confirms it.
- An ISR guard exclude-list has a known failure mode: every new state is a potential gap. If the project ever grows beyond the current state count, convert to include-list.
- Four bugs found this session, all the same shape - unguarded ISR re-entry under a flag that outlives the write that should have resolved it. The pattern is now fully internalized.

# 2026-07-16

**Morning:**
- Continued debugging. Started using bisection method by testing separate parts of the program. The BMP280_Poll is the problem. I2C driver is healthy.

**Evening:**
- Found several bugs, fixed them. Narrowed everything to just one function that breaks and one I2C part: pure transmit.

**Problems encountered:**
1. Bug: Intermittent TX_ADDR/index = 0 hang - root cause still open, systematic bisection in progress

**Symptom:** Intermittent hang with `hi2c.state == I2C_STATE_TX_ADDR`, `index == 0`, clean SR1, idle bus, no ER activity. Surfaces after variable time - sometimes seconds, sometimes minutes. Nine hypotheses tested and ruled out in one session.

**Bisection methodology:** Stripped the BMP280 layer entirely. Wrote a minimal test harness: tight loop calling `I2C_Master_Transmit_Receive` directly with the exact parameters as `BMP280_Measuring` - `send_length = 1`, `receive_length = 1`, status register `0xF3`, address `0x76`. `I2C_Process()` called inside the wait spin to advance state machine to DONE. UART counter printed each completed transaction.

**Result:** 24,000+ back-to-back N=1 transactions with zero gap, no gating logic, no `request_status` dance - zero hangs.

**Conclusion:**
- Bug does not live in i2c.c or the ISR chain independently.
- A more hostile timing pattern that `BMP280_Measuring` ever produces failed to reproduce the hang - the bug depends on something specific to the BMP280 state machine's interaction with the I2C driver.
- Scope narrowed: `bmp280.c` state machine logic - `request_status` gating, tick capture, measurement self-loop timeout, or the gaps between state transitions - is now the only remaining suspect.

**Next step:** Bisect the BMP280 layer itself. Remove pieces of `BMP280_Poll` incrementally to isolate which state transition or timing pattern is the actual trigger.

**Lesson learned:**
- When stuck after many failed hypotheses, bisection beats instrumentation. Simplify the system, not the observation.
- "It ran fine for a bit" is not a result. Cycle count is a result. 24,201 is a result.
- Clearing a suspect is as valuable as finding the bug. Nine ruled-out hypotheses plus one clean bisection result now bound the problem to a specific layer - that's not failure, that's the debugging process working.

2. Bug: TX BTF firing early - identical livelock shape as RX N>=3 bug, TX mode variant.

**Symptom:** Program hangs mid-transmission. Breakpoint on printf stops hitting. CPU captured is ISR re-entry storm. Secondary symptom observed earlier: mostly correct UART output with one scrambled fragment - same root cause, different timing: CPU spending almost all time trapped in re-entering ISR, occasionally letting main limp forward long enough to corrupt one printf mid-flight.

**Root cause at the register level:**
- In TX mode, BTF fires when the shift register has drained the last byte out onto the wire AND DR has not been refilled with the next byte yet - both registers empty simultaneously.
- For a 2-byte transfer at 16 MHz Sm-mode, the peripheral drains DR-then-shift-register faster than firmware refills DR via TXE. BTF fires at index == 1 (one byte early) before TXE gets a change to supply the second byte. This is apparently the normal timing outcome for this bus speed, not a rare race.
- BTF handler's `I2C_TX` case only had one condition: `index == TxLength`. At `index == 1` with `TxLength == 2`, no branch matched. No `else`. BTF flag never cleared. ISR re-entered immediately. Livelock.
- Identical bug shape to RX N>=3 mid-burst BTF - mutually exclusive dispatcher, one flag serviced per entry, no fallback for "flag asserted but firmware bookkeeping hasn't caught up".

**Fix:**
- Added `else` branch to `I2C_TX` BTF case: write next byte to DR (hi2c.Instance->DR = hi2c.pTxBuffPtr[hi2c.index++]), which both clears BTF and continues the the transmission.
- Before writing, check `index == TxLength - 1` - if this is the last byte, disable ITBUFEN first. Without this, a subsequent spurious TXE entry would fire after the last byte is written, since ITBUFEN would still be open.
- Sequence for 2-byte transfer confirmed correct: entry 1 - TXE writes byte 0 (`index 0->1`); entry 2 - BTF fires again (`index == TxLength`), original branch issues STOP, sets `I2C_STATE_FINISHING`; `I2C_Process()` calls `I2C_PollStopConfirmation`, reaches DONE.
- Fix generalizes correctly to any TxLength - fallback handles any mid-burst BTF, tail logic remains in the original branch.

**Validation:**
- Confirmed via GDB single-step: entry 1 TXE, entry 2 BTF fallback, entry 3 BTF STOP - exactly as designed.
- Stress test: repeated `I2C_Master_Transmit` alone, thousands of cycles, no hangs.
- Folded back into full `WriteCtrlMeas->Measuring` alternating test, then full `BMP280_Poll` cycle - confirmed stable.

**Lesson learned:**
- BTF in TX mode is not a "last byte" signal - it's a "both registers empty" signal. It can fire at any point where firmware falls one byte behind the peripheral. Never assume it only asserts at the end.
- The same dispatcher design flaw (mutually exclusive `if/else if`, no fallback when no branch matches) produces the identical livelock in both RX and TX paths. One architectural assumption - "this flag only asserts when I expect it to" - caused two separate bugs. Hardware does not respect firmware's timing assumptions.
- Scrambled UART output is not always a buffer or printf bug. If the CPU is spending 99% of its time trapped in an ISR storm, any printf that straddles the livelock onset will produce corrupted output. Blame the ISR before blaming the print.

# 2026-07-15

**Morning:**
- Debug continuation: found out the NVIC Priority bug. Fixed it.

**Evening:**
- Tested the program after the NVIC Prioriry fix - did not work.
- Continued debugging.

**Problems encountered:**
1. Bug: EV and ER interrupts both sitting at priority 0 - preemption model never existed in hardware.

**Symptom:** Entire documented architectural premise - "EV priority 38, ER priority 39, EV can preempt ER" - was never true in hardware. Those numbers were vector table positions, not execution priorities. At identical priority 0, neither handler can preempt the other. Once either starts running, it runs to completion regardless of what else becomes pending.

**Root cause at the register level:**
- Priorities were never explicitly set via `NVIC->IPR[]`. Both EV and ER defaulted to priority 0 after power-on reset.
- Vector table position determines dispatch order when both interrupts are simultaneously pending and neither is running - it has nothing to do with preemption. Preemption is controlled exclusively by `NVIC->IPR[]` values.
- At identical priority, if ER runs and holds a `CPSID I` critical section, everything is masked for the entire handler body - no priority-based partial masking to fall back on.

**Fix:**
- Set priorities explicitly via `NVIC->IPR[]`:
    - EV = priority 5 (higher, can preempt ER)
    - ER = priority 6 (lower)
- Shift value: `5U << 4` and `6 << 4` - STM32F4 implements 4 priority bits in the top nibble of each IPR byte, bottom nibble unused.
- Verified `NVIC->IPR` is declared as a byte array in the header (`IP[240]`), so `IPR[31]` and `IPR[32` correctly address IRQ 31 and IRQ 32 directly. Word-packed headers (`uint32_t[60])` would require `IPR[n/4]` indexing - wrong register entirely if confused.

**Lesson learned:**
- Vector table position != execution priority. Never confuse IRQ number with pereemption level.
- Default priority after reset is 0 for all interrupts - highest possible. If your architecture requires one interrupt to preempt another, you must set priorities explicitly. Assuming the hardware matches your design document is not verification.
- Always confirm NVIC->IPR array type in your header before writing priority values - byte array vs word-packed array produce completely different register targets for the same index.

# 2026-07-14

**Morning:**
- Continued debugging. Now there is a bus hung during Measuring phase: SB is fired, ADD phase never happens.

**Evening:**
- Continued debugging. Fount out the bug when during RX both BTF and RXNE are set mid-read since the firmware is not keeping up with the hardware.

**Problems encountered:**
1. Bug: ISR livelock - BTF firing mid-burst with no handler match
**Symptom:** EV handler re-entering itself hundreds of times per second. GDB shows CPU stuck at handler entry (`PC: 0x08000480`), never returning to main loop. SR1 = `0x44` - both BTF (bit 2) and RXNE (bit 6) set simultaneously. `hi2c.state == I2C_STATE_RX_ADDR`, `hi2c.index == 0`, `hi2c.RxLength == 24` (calibration burst read).

**Root cause at the register level:**
- ISR dispatcher is a strict `if / else if` chain - one branch per entry. BTF branch runs first, RXNE is never examined in the same entry.
- BTF handler for `RxLength >= 3` only has two cases: `index == RxLength - 3` and `index == RxLength - 2`. Neither matched at `index == 0`.
- No match -> switch falls through -> no DR read -> BTF and RXNE never cleared -> ISR returns with flags still asserted -> hardware immediately re-triggers the handler -> infinite livelock.
- Root assumption that was wrong: "BTF only ever asserts at the two tail-end bytes". That's a firmware assumption, not a hardware guarantee. BTF asserts wheneveer DR and the shift register both hold valid data - at any index - if firmware falls behind for any reason (debugger halt, priority contention, anything).

**Fix:**
- Added a third `else` arm inside the `RxLength >= 3` BTF case for mid-burst BTF: read DR into `pRxBuffPtr[hi2c.index]`, increment `index`, touch nothing else. No STOP, no ACK manipulation - just drain one byte and exit. Hardware moves shift register contents into DR, RXNE re-asserts, existing RXNE handler picks up the byte normally on the next entry.
- N = 2 (POS path) is immune - ACK already cleared and NACK committed before ADDR is released, so the second byte is NACKed by construction. No analogous fallback needed there.

**Lesson learned:**
- Hardware does not respect firmware's assumptions about when flags assert. BTF is a physical condition - DR and shift register both full - not a protocol-layer concept tied to "end of burst". Any mid-burst delay can trigger it.
- A dispatcher that takes exactly one branch per entry and does nothing when no branch matches is not safe - it silently converts an unrecognized flag combination into an infinite livelock with no visible symptom except "program stopped making progress".
- Always ask: if this branch matches but does nothing useful, does it at least clear the condition that triggered it? If not, the ISR will immediately re-enter.

# 2026-07-13

**Morning:**
- Started debugging.

**Evening:**
- Continued debugging.

**Problems encountered:**
- Debugging sucks. I love it though, because it sucks and it is hard.

1. GDB showing `OVR` flag set in SR1 and `hi2c.state == I2C_STATE_TX_ADDR` immediately at the first EV handler breakpoint after `load`. Spent time chasing OVR as a real bug.

**Root cause:**
- `load` in GDB/OpenOCD only rewrites flash contents. SRAM, CPU registers, and I2C peripheral registers are completely untouched. The previous debug session left the I2C peripheral in a broken half-transacted state and `hi2c` struct in SRAM with state values. `load` has no knowledge of any of that.

**Fix:**
- `monitor reset halt` before any debug session, not just `load`. That triggers a genuine core reset - re-runs startup code, zeroes `.bss`, reinitializes the peripheral registers from a known-clean state.

**Lesson learned:**
- `load` != reset. Never trust register values or global struct state after a bare `load` following a previously crashed or buggy session. Always reset before diagnosing. Chasing ghost state is the fastest way to waste debugging hours.

2. `I2C_PollHardwareBusy` had no timeout - a bare `while(1)` that spins forever if BUSY never clears. Entire interrupt-driven architecture collapses into a blocking spin at this one point.

**Root cause at the register level:**
- BUSY bit in SR2 stays asserted if the previous transaction's STOP condition didn't fully complete electricaly before the next transaction fired. PollHardwareBusy had no escape path - no timeout, no error reporting, no yield.

**Fix:**
- Added 4 ms timeout (not 3 ms - start tick can land one cycle before SysTick underflow, so one extra tick margin is required to guarantee minimum real wall-clock time).
- On timeout: write `error_code = I2C_ERROR_BERR` first, then assert SWRST (`CR1 bit 15`), then write `state = I2C_STATE_ERROR` last. Ordering matches the real BERR producer in `I2C_ER_IRQHandler` - the main loop's `I2C_Process` BERR branch assumes SWRST is already 1 when it sees that error code.
- Critical section (`CPSID I / CPSIE I`) wraps only the three writes - not the polling loop. Masking interrupts for the whole loop would mask SysTick, freezing the tick counter and making the timeout condition never true - an unconditional infinite loop with extra steps.
- NVIC ICPR clear not needed here - that clears a pendin EV interrupt latched mid-ISR. This path runs in main-loop context, no ISR was in flight.

**Lesson learned:**
- A single blocking helper can silently invalidate an entire non-blocking architecture. Every polling loop needs an exit condition that doesn't depend on hardware behaving correctly.
- Timeout budget must account for SysTick phase alignment, not just the nominal interval.
- Reusing an existing error code (`I2C_ERROR_BERR`) means inheriting all assumptions that code carries elsewhere. Match the full invariant - including SWRST state - or give it its own code.

# 2026-07-12

**Morning and afternoon:**
- Finished implementing BMP280 state-machine.
- Refactored busy-wait BMP280 functions to non-blocking.
- Adjusted the main loop to new BMP280 reality.
- Debugged the BMP280 driver.

**Evening:**
- Started debugging the I2C driver.

**Problems encountered:**
- Thought that even if the status-read transaction completes, the register `Status` of BMP280, bit 3 of it, is 0 (sensor genuinely finished converting - a legitimate success), but by coincidence this is also the poll cycle where SysTick_GetTick() - hbmp->measure_start_tick has just crossed the 15ms mark, the timeout is fired. But then realized that the timeout should only apply when the measurement is still ongoing, since the bit 3 is already telling decisively whether the sensor finished and there is no scenario where a completed measurement should also be judged late.
- BMP280_Init firing the same transaction repeatedly on every poll cycle instead of waiting for the result.

**Root cause at the register level:**
- Missing `hbmp->request_status = BMP280_REQUEST_FIRED` after the transaction was dispatched. The fired-flag guard at the top of the function never triggered, so every call to BMP280_Poll() fired a fresh I2C transaction directly into a busy bus.

**Lesson learned:**
- In a non-blocking state machine, firing a transaction and marking it as fired are not optional partners. If you dispatch without setting the flag, the next poll cycle has no memory that anything is in flight.

# 2026-07-11

**Morning:**
- Finished implementing Interrupt-Driven main loop with its error handling paths.
- Developed the state-machine for BMP280.c.

**Afternoon:**
- Started implementing the state-machine and refactoring the existing functions of the BMP280 driver.

**Evening:**
- Wrote a Linkedin post about ITBUFEN and my bug from yesterday.

**Problems encountered:**
- I thought the BMP280 driver would not require much changes. It seems I was wrong. State-machine + functions changes + polls :)

**Root cause at the register level:**
-

**Lesson learned:**
- Interrupts are hard. You will spend much more time designing the program than actually writing it :)
- I am using a state machine that depends on one I2C transaction per state. BMP280_TriggerMeasurements function had two I2C transactions. I have spent maybe more than an hour trying to grasp what do I need to do with that. 
I started to wrap the I2C transactions in different IFs, placed the I2C_STATE_DONE polling busy-wait loop after the first transaction. 
Only then I understand that I can split them in two functions and introduce another state to safely handle the interrupt-driven transactions.

# 2026-07-10

**Morning:**
- Finished implementing I2C_EV_IRQHandler.
- Debugged and fixed I2C_EV_IRQHandler.

**Evening:**
- Implemented interrupt-driven I2C_Master_Transmit and main loop (almost).

**Problems encountered:**
- Honestly, interrupts did not make much sense to me. But I've started grasping that topic. It is hard, but I love it!

**Root cause at the register level:**
-

**Lesson learned:**
- ITBUFEN does not change what SR1 reports. TXE and RXNE are set in SR1 by the shift-register/DR hardware logic regardless of ITBUFEN's value - that biy only controls whether the NVIC interrupt line gets pulsed for TXE/RXNE. So my attempt to use `(sr1_snapshot & (1 << 2) && ((cr2_snapshot & (1 << 10)) == 0))` to distinguish whether TXE/RXNE or BTF is really the cause of interrupt was not successful. 
So I removed the ITBUFEN check and just placed the BTF branch before the RXNE/TXE to prioritize the BTF when both it and the RXNE/TXE are set.
- STOP completion is not something the ISR can confirm in the same entry it writes the STOP bit - there is no "STOP was physically sent" interrupt, it has to be polled separately (SysTick-fenced timeout, checking hardware state).

# 2026-07-09:

**Morning:**
- Started implementing I2C_EV_IRQHandler and its state machines. I'm not quite sure what I am doing but it is ok :)

**Evening:**
- Continued implementing I2C_EV_IRQHandler: RxE and BTF state machines.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-08

**Morning:**
- Implemented GPIO Clock-banging sequence to drag the stuck peripheral through an infinite lock with SDA line held low.
- Implemented bit-banging STOP-issuing.
- Implemented a distiction between BERR recovery sequence and ARLO/AF recoveries. Since the BERR is a superset of ARLO and AF (if the bus has all three errors, handling BERR with SWRST clears all evidence of ARLO/AF and provides the ultimate recovery by reseting the peripheral and reinitializing it), we should handle BERR if available or if there is no BERR, ARLO/AF.
- Implemented the branching inside AF recovery sequence: if there are both ARLO/AF, then the MCU has lost its "controller status", so it is now a target. Targets perform "STOP conditions" by pulling off the lines letting them float. So the STOP issuing in the AF recovery is a redundant one if the MCU is a target.
- Fixed read-modify-write error flags clearing (hi2c.Instance->SR1 &= ~(1 << 9)). If hardware sets some other bit to SR1 in the handful of cycles between the read and the write-baco of one of those two RMW statements and the bit comes after the CPU has read the state of the SR1, but before it has written it modified back, CPU does not know that there is a new bit and that it should reread the register. It just modifies the old register value and writes it to the SR1. So the new bit is gone.
The solution: hi2c.Instance->SR1 = ~((1 << 13) | (1 << 9) | (1 << 5)). A single write to the SR1: bits 5 and 13 are reserved bits, writing 0 or 1 to them does nothing, but according to spect-compliance/MISRA hygiene we should write to them only their reset values (0x0000). 
Also bits 7, 6, 4 to 0 are read-only, so writing 1 to them does nothing. Other bits are write-0-to-clear, writing 1 to them has no harm whatsoever.


**Evening:**
- Started designing and developing the AF and ARLO main loop error branches with SWRST as a fallback path.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
-

# 2026-07-07

**Morning:**
- Finished designing the I2C_ERR_IRQHandler. Started implementing the Handler.

**Evening:**
- Implemented I2C_Reinit() function.
- Added Interrupt enablement to both I2C_Init and Reinit.
- Created the first skeleton of I2C1_EV_IRQHandler.
- Almost finished implementing the main loop ERROR branching with explicit I2C_BERR branch that handles the GPIO Clock-Banging.

**Problems encountered:**
- (None)

**Root cause at the register level:**
-

**Lesson learned:**
- BSRR register in GPIO: it allows to change the state of individual pins atomically (in a single CPU instruction cycle) without using a read-modify-write operation (ODR mechanics).
- NVIC's Write-1-to-clear ICPR (Interrupt Clear-Pending Register) and ICER (Interrupt Clear-Enable Register): a direct write assignment instead of a read-modify-write (|=) operation.

# 2026-07-06

**Morning:**
- Continued designing the I2C ER_IRQHandler: BERR recovery that requires SWRST, NVIC_ClearPendingIRQ (for possible EV_IRQHandler interrupt), GPIO_IDR to check during the I2C reset phase if the SCL and SDA pins are high or low (if high, then the bus is free, because the external pull-up resistors pulled the line high).

**Evening:**
- Designed the Clock-bang recovery sequence (manual toggling of SCL line through GPIO) to drag the external sensor out of the forever locked state when it holds the SDA LOW.

**Problems encountered:**
- It is not a problem, but I did not know that there are so many things you need to think about when designing an IRQ handler :)

**Root cause at the register level:**
- SWRST (Bit 15 in CR1) should be 1 during the Clock-banging recovery sequence to preven a chaotic and unpredicable internal state of I2C after returning to MODER AF.

**Lesson learned:**
- Bit-banging and Clock-banging recovery sequence.


# 2026-07-05

**Morning:**
- Reread the whole design implementation conversation with Claude who acts like a strict tutor who only guides me and makes me think about everything myself, with no answers or code snippets from its side.

**Afternoon:**
- Implemented struct handler.
- Started developing ISR handlers EV and ERR and their timing issues. EV (priority 38) can interrupt ERR handler (priority 39) mid-write, so ordering rule index-then-state does not work.

**Evening:**
- Wrote a LinkedIn post about Normal-to-Normal memory accesses are not guaranteed to arrive in order.

**Problems encountered:**
- Too many interrupt problems. I am tired of them xD
- Still have difficulties understanding e.g. I2C_EV interrupt that can interrupt I2C_ERR handler because of higher interrupt priority. But I am fighting.

**Root cause at the register level:**
-

**Lesson learned:**
- The transitioning from the wait busy polling to the interrupt-driven architecture - need to think about many things: timing, contexts, possible interruptions of other ISRs etc.
- Two I2C interrupts handlers with different priorities create interrupt mid-write problem.

# 2026-07-04

**Morning:**
- Finished developing the I2C thread-safety and receive sequencing for the Interrupt-Driven Architecture.
- Developed Transmit and Receive transaction types in Interrupt-Driver Architecture.
- Started developing the Master_Transmit_Receive's repeated-start handoff - the point where ISR is running in TX-phase and has to pivot into RX-phase without returning to the main loop in between.

**Evening:**
- Finished developing the Master_Transmit_Receive's repeated-start handoff.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
- Memory regions, types and attributes: memory system ordering of memory. It is fascinating that even Normal-to-Normal accesses are not guaranteed to be in order.
- DMB vs DSB: DMB ensures that ongoing memory transactions completely before  subsequent memory transactions, meanwhile DSB ensures that ongoing memory transactions complete before subsequent instructions execute. Their gigantic architectural costs difference: DMB takes roughly 1-2 CPU cycles on M4, DSB can take from 4 to 20+ CPU cycles depending on the bus congestion. For the time-critical ISRs, it is enormous difference and a gamechanger.
- Guaranteed minimum time: to guarantee that a least one full millisecond of real wall-clock time has physically elapsed, regardless of how badly the snapshot aligns with the clock phase (is was made 1 CPU cycle before underflow - the real time of 62.5 ns, or immediately after the reload - the real time 1 ms), the best solution is to always budget a minimum tick count of 2.

Guaranteed Minimum Time = (Budgeted Ticks - 1) * Tick Period.

So if the budget is 2 ticks: the worst-case is (2 - 1) * 1 ms = 1 ms minimum guaranteed wait.
- Within a single ISR execution, there is no way it can interrupt itself. For the I2C Event IRQ Handler at this point of the program development, there is no second reader that can observe an incomplete state update, the main loop is frozen, so no danger is around. 
Also noted: there is a hazard posibility in the future code expansion: when we add DMA, the DMA countroller could read phase independently of the ISR, so that 1-2 cpu clock cycle window between the index update and the phase update could bring problems. For now there is no current race, but preserved ordering discipline is case a second bus master is introduced.
- Ordering discipline: index-then-state - introduced because the main loop could be caught mid-read by an ISR firing at an arbitrary instant.

# 2026-07-03

**Morning:**
-

**Evening:**
-

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
- 

**Lesson learned:**
-

# 2026-07-02

**Morning:**
- Continued to develop the Interrupt-Driven Architecture for I2C driver.

**Evening:**
- Continue developing and tracing possible race conditions, parts of code that requires critical sections and interrupt prioriries of ISR.

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

**Lesson learned:**
- Interrupts are hard. You cannot guarantee that everything would happen just like you predicted, because you never know when one interrupt would stretch a bit and then would be interrupted by the interrupt with higher priority (lower number = higher priority). 

# 2026-07-01

**Morning:**
- Finished debugging the I2C BMP280 Temperature Pressure Readings program. Fixed the last remaining problem. Now it works, printing the readings in human-readable format over UART with 115200 baud rate.
- Wrote a LinkedIn post about RXNE vs BTF for the final byte in N >= 3 receive in I2C.

**Evening:**
- Started developing the Interrupt-Driven Architecture for the I2C BMP280 Temperature and Pressure Readings project.

**Bugs encountered:**
1. Back-to-back I2C transactions failing due to STOP not completing before next START.
**Symptom:** CDB stalls at line 282 in I2C_PollHardwareBusy immediately after entering I2C_Master_Transmit during BMP280_TriggerMeasurements execution. The function spins forever on SR2 BUSY bit never clearing. Minicom silent because no measurement has been triggered and made.

**Root cause:** 
1.Software: After successfully finishing the calibration data issuing over the lines and its reconstruction, the CPU initialized the UART driver and entered the TriggerMeasurements function. The bus was still BUSY. After receiving the last N-1 byte during the calibration data receiving, the function did not wait for the BUSY condition being detected by the hardware. It exited immediately.

2. Electrical: After STOP is generated, BUSY clears only when SDA and SCL both return to their HIGH state. On my breadboard, I have 4.7 kOhms external resistors and 10 kOhms external resistor on the BMP280 chip. The rise time of an open-drain line is goverened by RC - the pull-up resistance times the bus capacitance. The resistance in parallel equals (10 kOhms * 4.7 kOhms) / (10 kOhms + 4.7 kOhms) = approximately 3.2 kOhms. 
RC = 3.2 kOhms * 100 * 10^(-12) = 0.32 microseconds = 320 ns. 1 CPU's cycles = 1 / 16 000 000 = 62.5 nanoseconds.

So 320 nanoseconds / 62.5 nanoseconds = approximately 5 CPU clock cycles until both the lines are HIGH.

**Fix:** After the last N-1 byte has been read from the DR, I added the loop while (I2C->CR1 & (1 << 9)) to wait until STOP bit in CR1 is cleared by hardware that means that the STOP is detected and the lines have returned to idle. Only then return from the HAL function.

**Lesson learned:**
- Register writes that generate bus events are not instantaneous. The gap between software writing a control bit and the physical electrical consequence completing is governed by bus physics, not CPU speed. Any function that generates STOP must confirm STOP completion before returning - otherwise the next transaction races against an electrically busy bus.
- Interrupt-Driven Architecture is hard to grasp and implement. But it is wonderful, because it sucks. So that's why I need to do it :)

# 2026-06-30

**Morning and evening:**
- Debugging of the I2C Temperature and Pressure readings.

**My thoughts about debugging:**
- Too much hung poll loops xD 
- Too much moments in which nanoseconds and microseconds made a difference.
- Now it breaks on BMP280_TriggerMeasurements function.

**Bugs encountered:**
1. CSB/SDO floating pins causing erratic bus behavior

Not specifically a bug because I fixed it before even flashing the program to the BlackPill. But simultaneously if I did not find out that I need to connect the CSB to 3.3V rail to get I2C mode and the SDO to GND to get the address 0x76, then I would have had the BMP280 chil that acts for some time like I2C chip, then like SPI chip, or even better: the I2C transactions would have ended with errors due to the fact, that no chip whatsoever has the address 0x76 (if at the moment the BMP280 had its SDO high, so the address 0x77).

In my mental model, I somehow omitted it and only debug session allowed me to fully grasp that if the BMP280 has 6 pins, all 6 of them are in use.

2. RXNE vs BTF for the final byte in N >= 3 receive.
**Symptom:** GDB stuck at the line 241: I2C_PollHardwareFlags called in I2C_Master_Transmit_Receive function during BMP280_Calibration.
RXNE never fires. Minicom silent since BMP280_Calibration is a part of BMP280_Init that should complete before UART2 initialization, so UART2 had never had an ocassion to initialiaze.

**Root cause:** RXNE signals that a byte has been transferred from the shift register into DR - but for the final byte, the master issues NACK, which tells the slave to stop driving SDA after the 8th bit, so the 9th clock pulse produces no valid data transition, the shift register never completes a transfer, and RXNE never asserts.

**Hardware consequence:** SCL is held low by the peripheral's clock stretching mechanism while the master polls indefinitely - the bus freezes.

**Fix:** For the final two bytes of an N >= 3 receive, BTF must replace RXNE - BTF fires when both DR and the shift register are simultaneously full, which happens before the ACK/NACK decision is made, giving software the deterministic window to clear ACK, read DR, set STOP, and read the final byte directly from DR without any further polling.

3. ACK clear timing relative to shift register state
**Symptom:** The CDB stalls at the BTF poll for byte N-2, not even reaching the DR read. The minicom still silent because UART has never been initialized yet.

**Root cause:** After the fix of the bug 2, the ACK bit clearing stays immediately after the main loop 0...N-3 exits - immediately after reading DR for byte N-3 (the last byte handled by the RXNE loop).
The I2C needs 90 microseconds for the byte (8 bits + 1 ACK/NACK). The CPU executes a single instruction in 62.5 nanoseconds. Between the last byte reading of the byte N-3 and the ACK clearing elapse 2-4 CPU cycles. 4 CPU cycles = 250 ns.
During that 250 ns the I2C pulled over the lines: 250 ns / 10 000 ns per bit = 0.025 bit (2.5% of the first bit out of 9). It means that ACK clearing arrives essentially at the very start of that byte's transfer, not after it. It is too early.
Clearing ACK here causes the master to NACK byte N-2 itself, not byte N-1 as intended.

**Fix:** The reference manual for STM32F411 presents a general sequence for N > 2 byte reception, from N-2 data reception:
• Wait until BTF = 1 (data N-2 in DR, data N-1 in shift register, SCL stretched low until data N-2 is read)
• Set ACK low
• Read data N-2
• Wait until BTF = 1 (data N-1 in DR, data N in shift register, SCL stretched low until a data N-1 is read)
• Set STOP high
• Read data N-1 and N.

For my receive transmission (0...N-2) for bytes N-2 and N-1 it simplifies to:
• BTF wait
• ACK clear 
• DR read (byte N-2) 
• STOP
• DR read (byte N-1).

**Lesson learned:**
- Debugging is hard. Maybe it is the most time-consuming activity in embedded. You need to trace down what does not work. And it sucks :) Love it :)
- CSB and SDO must be connected to VCC/GND depending on what configuration you need. With these pins floating, the bus behaves erratically and unpredictably.
- The ACK/NACK decision for a byte is made at the transition point - the moment that byte moves from the shift register into DR (freeing the shift register for the next incoming byte). That's when the 9th clock pulse fires and the hardware samples whatever value is currently in the ACK bit. This is also why the 0 to N-3 loop never needed to think about ACK explicitly - it defaults to 1 (set during init), and every byte transitions shift-register-to-DR with ACK=1 automatically, requesting the next byte.

# 2026-06-29

**Morning:**
- Implemented BMP280 Init function.
- Developed and implemented a testing loop inside main.c.
- Fixed all the linking issues before the flashing to the BlackPill. 

**Evening:**
- Dismantled the previous circuit on the breadboard.
- Constructed a new circuit for th project testing:  BlackPill's B6 (SCL) connected to row 42, B7 (SDA) connected to row 43. The BMP280 sits on rows 40-45 (40 - VCC, 41 - GND, 42 - SCL, 43 - SDA, 44 -CSB, 45 - SDO)). before SDA and SCL wires on rows 42 and 43 (before the BMP280) there are two resistors 4.7K ohms connected SCL to 3.3v power rail and SDA to 3.3v power rail. CSB connected to 3.3V because it is I2C, SDO is connected to GND rail because the address is 0x76.
The UART-USB adapter has 4 pins (VCC, GND, TXD, RXD). GND is connected to GND rail. TXD is connected to PA3 (RX2) breadboard row. RXD is connected to PA2 (TXD) breadboard row. VCC is not connected because the adapter connected by USB.

**Problems encountered:**
- uint8_t etc. appeared undefined after assemlying the entire program. Changed the compiler in "c_cpp_properties.json" from "clang" to "arm-none-eabi". A trivial issue, but it was suprisingly to me that `uint8_t` etc can be undefined :)
- Other errors during compilation process. Fixed all of them by including missing header files or standard libraries.

**Root cause at the register level:**
-

**Lesson learned:**
- `<inttypes.h>` and portability solution to a potential cross-compilation problem: PRId32 and PRIu32.
- Linking stage may be even trickier that the process of writing the program.

# 2026-06-28

**Morning:**
- Wrote the JOURNAL entry for 27.06.
- Implemented BMP280 Calibration function.
- Developed and implemented SysTick reusable driver to use it in the BMP driver functions (BMP280 Trigger Measurements function).

**Evening:**
- Implemented BMP280 Trigger Measurements function.
- Implemented BMP280 Read Measurements function.
- Implemented BMP280 Temperature and Pressure Compensate functions, alongside with the high-level BMP280 Calculate Data function that manages the Compensate calculations.

**Problems encountered:**
- Variable-Length Arrays: did not know that the C standard strictly forbids initializing a VLA at the moment of declarion.
- Forgot for a moment that in C arrays are initialized using curly braces {}, not square brackets :)

**Root cause at the register level:**
-

**Lesson learned:**
- BMP280 Driver: the necessesity of timeout handling if the status register is not changing to the "Measurement done!" state.
- Variable-Length Arrays and how to use them. Runtime length variable are not allowed to use with the VLA (uint8_t size = 2), so I needed to use compile-time constant (const uint32_t size = 2).
- Reminded myself how to initialize the arrays. It is trivial, but even that is forgottable a bit from time to time in a midst of new informations. 

# 2026-06-27

**Morning:**
- Implemented the header file bmp280.h.
- Started developing the driver file bmp280.c with Claude as my strict tutor who does not give me code but navigate me and lets me find answers myself.

**Evening:**
- Analyzed how to create embedded posts on Linkedin, how often (twice a week), how much time should I spend for a one post (10-20 minutes). I do not need to become a content maker, I have totally different goal. I just need to be consequent in posting twice a week about interesting things in my projects or about things I found hard to understand and grasp.

**Problems encountered:**
- I'm still not get used to the fact that I cannot just write registers from memory map to manipulate the BMP registers :) Because all I can do using it is to manipulate the Blackpils registers, not the external peripheral's one.

**Root cause at the register level:**
- BMP280 is an external chip. I have direct access only to the STM32F411 BlackPill. With BMP280 I should manipulate through that BlackPill and I2C protocol communication.

**Lesson learned:**
- Every communication, even a simple polling of a bit inside a particular register, in BMP280 requires I2C transaction. It is heavy xD

# 2026-06-26

**Morning:**
- Refactored all polling loops in the I2C driver by making it a separate function, optimizing all register reads to single-write and preventing the POS and ACK from clearing and re-enabling during Write trasactions.
- Developed a timeout SysTick mechanism via function pointer to provide abstraction to hook up different types of counters. SysTick timer is set for the next verstion, when I am going to move my poll loops to interrupt-driven solution.

**Evening:**
- Discovered that I need to take snapshots of what I am doing and use them in my upcoming Linkedin posts.
- Fixed lack of PE disenablement in I2C_init to prevent setting the I2C register bits with possibly ON PE bit after SWRST recovery etc.
- Explicitly cleared PUPDR register bits in I2C_init to make sure they are unset.
- Started developing the architecture of the BMP280 driver.

**Problems encountered:**
- 

**Root cause at the register level:**
-

**Lesson learned:**
- Single-Write vs. Read-Modify-Read. For the register like SR1/SR2 with its Write-0-to-Clear protection. Single-Write is wonderful.
- PE bit should be cleared explicitly in the top of the init function, not just be assumed to zeroed out after reboot etc.

# 2026-06-25

**Morning:**
- Implemented 2-byte receive I2C function with POS.
- Simplified "dummy" readings in order to clear the flags: if the register is read in a poll, there is no additional redundant read etc.

**Evening:**
- Rewrote polling functions to achive better unification and reusability.

**Problems encountered:**
- I am clearing the POS bit and re-enable the ACK bti unconditionally in the errror return ways. I guess I need to clear the POS only if it is set and re-enable the ACK only when it disabled.

**Lesson learned:**
- POS=1 shifts the ACK/NACK control point from RxNE (current byte in DR) to BTF (next byte in shift register). This is what makes it possible to NACK byte 2 while still receiving byte 1, without the CPU missing the timing window.

# 2026-06-24

**Morning:**
- Implemented I2C_Master_Transmit_Receive function.

**Evening:**
- Started debugging the driver. Encountered the POS bit in CR1.

**Problems encountered:**
- Realized that for 2 byte transaction, I need to involve the POS bit and its mechanics.

**Lesson learned:**
- The POS bit in CR1. Honestly, I did not know that it is cost-effective to handle separately not only one-byte receive transaction, but also two-byte receiving transaction.

# 2026-06-23

**Morning:**
- Finished I2C_Master_Transmit and implemented I2C_Master_Read functions.

**Evening:**
- Developed the I2C_Master_Write_Read (Master_Transmit_Receive) function. Implementation is next.

**Lesson learned:**
- I2C functions development.

# 2026-06-22

**Morning:**
- Implemented I2C initialization function.

**Evening:**
- Enhanced the init function.
- Started implementing Master_Transmit function.

**Problems encountered:**
- Difficulty to grasp how the I2C files and BMP280 files are connected since we are not writing the register of BMP280 like we are doing it with the STM32.

**Lesson learned:**
- I2C Registers.

# 2026-06-21

**Morning:**
- Finished developing the I2C Thermometer and Barometer Readings project.
- Started implementing the I2C driver.

**Evening:**
- Reread all the implementation process and details.

**Problems encountered:**
- Too much information :) It is awesome

**Lesson learned:**
- BMP280 transactions.

# 2026-06-20

**Morning:**
- Understood the mechanism behind SCL and SDA lines.

**Morning and evening**:
- Continued developing the I2C Thermometer and Barometer Readings project.

**Problems encountered:**
- (None today) etc

**Lesson learned:**
-

# 2026-06-19

**Morning and evening:**
- Continuing the development of the I2C Thermometer and Barometer Readings: timing and BMP280 registers.

**Problems encountered:**
- Well, it is not a problem, but it is hard. I love it :)

**Lesson learned:**
- BMP280 Bosch datasheets.

# 2026-06-18

**Morning:**
- Wrote the I2C type definition struct to my custom stm32f411 header file.
- Started developing the I2C Temperature and pressure project.

**Evening:**
- Continued the project development.
- Understood and calculated CCR and TRISE.

**Lesson learned:**
- I2C CCR and TRISE - timing registers.

# 2026-06-17

**Morning:**
- Soldered all new arrived chips.

**Evening:**
- Started developing the BMP280 Temperature and pressure reading project.

**Lesson learned:**
- BMP280.

# 2026-06-16

**Morning and evening:**
- Learned info about I2C: basic theory, MOSFET, edge cases, buses, logic analyzers, bit loop etc.
- Learned basic inforation about Half-duplex.

**Problems encountered:**
- A lot of information :) It is awesome, but a bit hard xD

**Lesson learned:**
- I2C.
- Buses.
- Half-duplex.

# 2026-06-15

**Morning:**
- Wrote a README.md for the ADC+DMA Sampling project.

**Evening:**
- Reread the README.md, fixed typos.
- Added the JOURNAL logs 08.06 - 15.06.

**Problems encountered:**
- I do not remember exactly what have I done a week ago lmao

**Lesson learned:**
- I need to write the Logs every day, not once a week, because I do not remember exactly e.g. what problems I had 6 days ago etc.

# 2026-06-13 & 2026-06-14

**All day** 
- Program revision. Logic understanding.
- Debugging on the breadboard with the Black Pill and a potentiometer.
- Some fixes intergration.

**Problems encountered:**
- The project has grown more than I expected :) Very many hidden details to know and notice. But it is how it is.

**Lesson learned:**
- Hardware bare-metal debugging.

# 2026-06-12

**Morning:**
- Finally finished the IRQ handlers. The project is basically done.

# 2026-06-11

**Morning:**
- Impemented ISRs (Interrupt Service Routine handlers) for the DMA1 and DMA2.

# 2026-06-10

**Morning:**
- Made the UART2 reusable driver more modular, developed ISR for DMA2_Stream0 with the Claude.

**Evening:**
- Implemented the DMA2 interrupt function and started writing the main loop.

# 2026-06-09

**Morning:**
- Continued implementing initialization functions in ADC+DMA Sampling project.

**Evening:**
- Finished ADC_init function.

# 2026-06-08

**Morning:**
- Started implementing the ADC+DMA Sampling projects: reread all architectural details and started implementing clock enabling mechanism.

**Evening:**
- Wrote the Clock initialization function

**Lesson learned:**
- ADC+DMA architectural details.

# 2026-06-07

**Morning:**
- Finished the potentiometer sampling project development with Claude as my strict tutor (that does not give me any code, just challenges me) - 9 hours in total.

**Evening:**
- Updated the Bought electronics list.
- Updated the JOURNAL adding logs 04.06 - 07.06.

**Problems encountered:**
- In general, no problems. But this development architecture session shows me that in automotive embedded writing code is the easiest part :) It is awesome!

**Lesson learned:**
- Architectural development.
- ADC + DMA.

# 2026-06-06

**Morning:**
- Learned theory about both the ADC and DMA.
- Started development of the potentiometer sampling project using the Claude Pro as my strict tutor who navigates and challenges me.
- Implemented the DMA and ADC typedef structs in my core header file.

**Problems encountered:**
- This project requires quite many registers and much knowledge. I like it :)

**Lesson learned:**
- ADC.
- DMA.

# 2026-06-05

**Morning:**
- Wrote READMEs.md for the Input Capture (project and reusable driver).

**Evening:**
- Bought necessary components for upcoming topics (I2C, SPI, ADC+DMA etc).
- Bought Claude Pro subscription.

**Problems encountered:**
- Realized I do not have a potentiometer for the ADC+DMA Sample project.

# 2026-06-04

**Morning:**
- Added a 104 (100 nF) ceramic capacitor in order to create RC filter to filter out mechanical switch bounces.
- Tested the circuit with a multimeter.
- Tested the program.
- Learned basic information about the retargeting system-layer functions.
- Made up the logs in the JOURNAL, describing all the events/details from the last 4 days.

**Evening:**
- Creating the reusable drivers structure with all completed drivers.
- Wrote READMEs.md for the PWM (project and reusable driver).

**Problems encountered:**
- None

**Lesson learned:**
- RC filters.

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
 
