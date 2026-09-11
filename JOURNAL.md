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

# 2026-09-11

**Morning:**
- Added new info to the JOURNAL logs for 05.09, 06.09.
- Completed the JOURNAL log for 07.09.
- Started the JOURNAL log for 08.09.
- Completed the JOURNAL logs for 08.09, 09.09 and 10.09. All bootloader-project JOURNAL logs are completed.

**Afternoon:**
- Wrote a README.md for the bootloader project.

**Evening:**

**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-10

**Morning:**
- Finished debugging the bootloader application: found the reason it was not working (I have messed up with the submodules).
- Tested the program on the breadboard.
- Completed the JOURNAL log for 31.08.

**Afternoon:**
- Completed the JOURNAL logs for 01.09 and 03.09.
- Completed the JOURNAL logs for 04.09 and 05.09. 
- In a halfway to complete the logs for 06.09.

**Evening:**

**Problems encountered:**
**1. Bug: Unshifted FLASH_SNB2 Enum value - the real bug that has caused these problems:**

*Symptom, after ruling out the tooling issue:* With a genuinely fresh, healthy debug session, signle-stepping through flash_erase() and continuing past the call to flash_bsy_checking() never reached a breakpoint set immediately after it - a real, reproducible hang, confirmed by testing with a full-speed continue.

*Root cause:*
The direct register inspection at the line setting STRT (FLASH->CR |= (1UL << 16U)) and printing the content of FLASH->CR in binary showed: 1000000011. The bit 0 (PG) and bit 1 (SER) were set simultaneously. 
Traced this back to the FLASH_SNB2 enum value, which still had been defined as bare 0x2 rather than pre-shifted into the SNB field's actual bit position (6:3). Since `sector_num` gets OR'd directly into FLASH->CR at whatever bit position it already holds, an unshifted value of 2 lands on bit 1 (SER) rather than anywhere in the SNB field at all.

*Fix:* Redifined the sector-number enum values with the shift baked in at declaration(FLASH_SNB2 = (2 << 3) and so on), so the value already sits in the correct field position before it's ever ORed into FLASH->CR.

*End-to-end confirmation:*
Then I ran the Pythong script fresh, with no debugger attached, board power-cycled clean. Script printed a clean success message with no NACKs or timeouts along the way. LED on the target board physically started blinking - script reported success and there was an observable hardware behavior.


# 2026-09-09

**Morning:**
- Derived and implemented assembling of 133-byte packet.
- Designed the sending and handling the response stage.
- Implemented the sending and handling the response mechanism.
- Designed and implemented sending the sentinel and waiting for the final confirmation.

**Afternoon:**
- Started testing and debugging + learnt how to run Python programs.
- Continued debugging - hit a core lockup state.

**Evening:**
- Continued debugging - fixed the core lockup state - it was OpenOCD/GDB tooling artifact.

**What was done:**
**Python Host-Side Uploader Script (Part 2):**

*Step 3 - checksum and packet assembly:*
Device side computes covered_sum as a uint16_t - every intermediate addition wraps at 16 bits silently in C. Python integers never wrap on their own.
Confirmed modular arithmetic property (a+b+c) mod N == (((a mod N) + b) mod N + c) mode N holds, meaning masking once at the very end (& 0xFFFF) after summing everything gives an identical result to wrapping after every add on the device - no need to mask per-term.

First checksum attempt used `content[0]` (the file's actual first byte) instead of 0xAA (the protocol's fixed Start_byte constant) - an accidental substitution of the wrong variable that happened to compile without error. Caught by tracing that content[0] never changes across loop iterations while the actual device-side checksum uses the fixed framing byte, not file content. Fix: changed to 0xAA.

Confirmed Python 3's sum() works directl on a bytes object, iterating it as plain int (0-255) with no conversion step.

Packet assembly: bytes([0xAA, 128]) + new_chunk + bytes([high_sum, low_sum, 0xBB]) - needed to learn that bytes only concatenates with another bytes-like object, never a bare int (bytes + int raises TypeError), fixed by wrapping every lone int in bytes([...]).

First version of the packet-building loop build packet fresh every iteration but never appended it anywhere - meant only the last chunk's packet survived past the loop. Same category of bug already caught once with chunks.append(). Fix: added packet.append(packet).

*Stage 4 - send/receive loop with retry semantics:*
Built a byte->function dics (NACK_ACTIONS) mapping device response values to host actions, after reasoning that three of the six response values (UART_NACK_RETRY, UART_NACK_CORRUPTED, UART_NACK_FLASH) all resolve to the identical host action ("resend this packet"), and two others (UART_NACK_ABORT, UART_NACK_OVERFLOW) bith resolve to "stop entirely" - a dict avoided writing the same handling logic three times over in an if/elif chain.

Retry function (retry()) is an empty no-op - since the loop's index only advances on a real ACK, "don't advance" is already the loop's natural behavior for any non-ACK response; the function exists only to be a valid dict entry. Abort function initially planned to return `return None` as its "stop" sentinel - caught before writing it that an empty-bodied Python function implicitly returns None too, meaning the retry function and the abort function would be indistinguishable by return value. Switched the abort sentinel to the string 'abort', a value nothing else in the flow could produce by accident.

Discovered mid-build that a `for i in range(total_chunks)` loop structurally cannot support "resend the same index" - `for` always advances its loop variable every pass, with no mechanism to hold it still. Converted to while j < total_chunks, incrementing `j` only inside the ACK branch.

First full draft of this loop accidentally kept all the Stage-2/Stage-3 chunk-building code (slicing, padding, checksum, packet assembly, `.append()`) sitting inside the same while body as the send/receive logic, reusing the same loop variable `i`/`j` for two conflicting purposes - "which packet to build next" and "which packet index to send/retry". This caused `packets` and `chunks` to grow unboundedly on every retry pass rather than being built once. 
Fix: separated the two loops entirely: a `for i in range(total_chunks)` that builds the complete packets list once, finished before any sending starts, followed by a separate `while j < total_chunks` that only ever reads `packets[j]`.

Host-side timeout-vs-NACK distinction: decided a device-silence timeout needs its own separate counter from the device's own internal retry counters, since the device's counters only exist and increment in response to bytes it actually receives - if the link or device is dead, the device has no idea a host is even trying to talk to it, so nothing on that side is counting anything. 
Chose 2 total attempts (1 initial + 1 retry) before giving up on silence specifically, reasoning that a second attempt has a real chance of recovering from a transient USB-serial hiccupt, but a third attempt after two consecutive silences adds nothing. Timeout counter resets on any response at all - ACK or NACK - since receiving anything at all proves the device is alive; it's specifically the "went completely silent" failure mode being guarded against, not general packet failure.

Several syntax corrections: Python has no `++`/`--` increment operators at all (timeout_counter++ and i++ both invalid - replaced with += 1), and confirmed int.from_bytes on a single byte gives an identical result regardless of the required byte-order argument ('little' vs 'big'), since there's nothing to reorder with only one byte.

*Stage 5 - sentinel packet and final confirmation:*
Sentinel confirmed to be genuinely 2 bytes only (Start_byte, Payload_len = 0) by re-checking the device's own case block - UART_PAYLOAD_LEN sets is_last = 1 and ACKs immediately on reading a zero length, without ever transitioning into the payload-reading state, so nothing else on the wire is expected or consumed for that packet.

Decided the sentinel confirmation should be a single-shot attempt with just a print statement on failure, not the full retry machinery built for real chunks - reasoned that by the time the sentinel is sent, every real chunk has already been successfully flashed; nothing left to corrupt, so the proportional amount of engineering here is much less than for the data-carrying chunks.

First draft called `int.from_bytes(ser.read(1))` directly, feeding the raw read result straight into the int conversion before checking whether it was empty - same emptiness-check-before-conversion issue already caught and fixed earlier in the timeout logic for real chunks. 
Fix: check and then convert - test `if not sentinel_response` first, only call `int.from_bytes` inside the non-empty branch. Also caught a syntax error from an incomplete comparison (`elif sentinel_response != :`) missing its right-hand operand - filled in with 0, maching UART_ACK_OK's real value.

*Final clean-up:*
I removed the `chunks` list entirely once confirmed it was write-only (appended to but never read anywhere in the finished script) and removed an unused fallback lambda in the `NACK_ACTIONS.get()` call, since the enum is closed at values 0-5, 0 is intercepted before the dict lookup ever runs, and 1-5 are all explicitly mapped - meaning `.get()` can never actually return the fallback on real hardware. 

**Application Image confusion:**
I initially misunderstood the entire point of the bootloader/UART transfer - I had assumption that the bootloader's own compiled binary was somehow "the chunks" being passed around, and that packets needed to be constructed by hand from nothing. 
Had to walk back to first principles:
the bootloader and the application are two entirely separate compiled programs, built from separate source trees, separate linker scripts, living at two different flash addresses (bootloader at 0x08000000/sector 0+1, application at 0x08008000/sector 2). The UART transfer's entire purpose is delivering a fresh copy of the application binary into flash at the application's address, without needing a debugger physically attached - the bootloader is the receiving/writing mechanism, not a payload.

I mistakenly ran the bootloader's own program.bin through the transfer at first - traced this by noticing the bootloader's own Makefile wrote to 0x08000000 (matching flash/flash-st targets and the linker script's ORIGIN), confirming that particular .bit was the bootloader itself, not an application payload, and definitely the wrong thing to send since it would try to overwrite the very code currently executing the transfer.

Took previously built (back to the beginning of my bare-metal journey) minimal LED-blink application project specifically as the payload: its own main.c, its own .ld file with ORIGIN = 0x08008000 instead of 0x08000000 (one-line change from the bootloader's linker script), and initially left LENGTH = 512K in that linker script unchanged - recognized this was too generous, since the application only legitimately owns Sector 2 (16KB), and a future application that actually approached that size would silently link fine while the bootloader's own `all_payload_bytes > 16384` runtime check would reject it at transfer time rather than the build catching the mistake earlier.
Fix: changed LENGTH to 16K to match the real sector boundary, so the linker itself would refuse to link an oversized image with a build-time error rather than deferring the failure to runtime.

**Bench test:**
**1. UART_ACK_START handshake bug at the 500ms erase stall:**
*Symptom*: every single script run - regardless of the .bin used - reported an immediate timeout waiting for the very first response byte, even the widened 500ms wait for the newly-added start-of-transfer signal.

Root cause: uart_chunk_receive_protocol()'s original first three lines were flash_erase(FLASH_SNB2), SysTick_Init(...), usart2_init() - it that order. 
flash_erase() calls flash_bsy_checking(), which spins until the sector erase physically completes - up to 500ms worst case for 16 KB sector erase at PSIZE=x32. During that entire window, usart2_init() had not run yet - USART2 was completely unclocked and unconfigured. Any byte arriving on the wire during that time was not buffered anywhere; it simply vanished, since the peripheral receiving it did not exist from the CPU's point of view.

*Fix, part 1:*
Reordered uart_chunk_receive_protocol() to call usart2_init() first, before anything else - confirmed safe since usart2_init() depends only on RCC/GPIOA/USART2 registers, and flash_erase() depends only on FLASH registers; the two touch completely independent peripherals with no ordering dependency between them.

*Fix, part 2:*
Even with USART2 clocked and configured before the erase starts, the erase itself still blocks the CPU for up to 500ms while flash_bsy_checking() spins. A byte arriving during that window would be received into DR by hardware (the peripheral genuinely is listening), but the CPU has no opportunity to read it out before a second byte could arrive and overrun/overwrite the first - since the protocol is fully polled, not interrupt-driven, and the CPU is busy-spinning somewhere else entirely.

Decided the correct fix wasn't a timing guess on the host side (waiting some arbitrary "long enough" delay before sending), but a real handshake: the device sends an explicit "ready now" signal to the host only once the erase has genuinely finished. 
Added a new enum value UART_ACK_START = 6 to UART_HostConfirmation_t, sent unconditionally right after flash_erase() returns (not before - the "safe to send" promise only becomes true once the erase is actually done). Confirmed a distinc value was the right call rather than reusing UART_ACK_OK - collapsing them would make "ready for chunk 0" signal indistinguishable from "chunk N successfully flashed" if ever logged or debugged later.

Deliberately did not add an error-check path around the erase step (no dedicated NACK for erase failure) - I have scoped it as out-of-scope for v1, consistent with the earlier decision to skip exhaustive fault coverage.

Host-side change: I have added a standalone, one-time wait for the UART_ACK_START byte before the main chunk-sending loop even starts, using a deliberately larger timeout (500ms, matching the erase's worst case) rather then reusing the 30ms per-chunk, since 30ms was sized for a completely different wait (post-chunk-transmission response time).

**2. False Lockup:**
*Symptom, after the UART_ACK_START fix was in place:* GDB sessions repeatedly showed me that the core is in a lockup state - `pc: 0xFFFFFFFE`, `msp: 0xFFFFFFFC` - and reading the vector table at `0x08000000` showed erased flash (`0xFFFFFFFF`) even immediately after a `load` command reported successful writes. `hardfault_dump` read all zeros, meaning `HardFault_Handler_C` has genuinely never executed even once - ruling out a real scenario that there was a fault inside a fault (double-fault scenario), since that would still have left the first fault's data written before things went wrong.

*Investigation:*
Confirmed STM32_Programmer_CLI's own verify path (via make flash) read back correct data immediately after writing - meaning real bytes really were landing in flash.
Yet the very next GDB attach, reading the identical addresses, showed erased flash again. Two tools disagreeing about the same physical memory - something was wrong with the debugging session. 

Killed the stale OpenOCD process entirely and started a completely fresh instance after `make flash` had already finished - the very first halt message from that fresh session showed a plausible, real `msp` value and `Handler HardFault` mode, in contrast to every prior garbage reading. 
Re-checking `hardfault_dump` right after a `load` (without an intervening reset) showed large nonsensical numbers - traced this to stale SRAM content from before Reset_Handler had run and zeroed .bss, not a genuine fault report; a reminder that RAM-resident state should never be trusted without a reset first.

A subsequent fresh halt, after a proper reset, finally showed a completely healthy signature: `pc: 0x080001ac` (matching the `load` command's own reported start address) and `msp: 0x20020000` (exactly SRAM_START + SRAM_SIZE, the correct default stack top). Re-reading the vector table in the same clean session showed correct, real values (0x20020000, 0x080001ad with the Thumb bit set) - confirming the firmware, the flash write, and the reordering fix had never been actually broken. Every earlier lockup/garbage reading had been an artifact of a debug session.


**Problems encountered:**
- (None today) etc

**Root cause at the register level:**
-

# 2026-09-08

**Morning:**
- Derived and implemented the error handing in the main() function.
- Built a breadboard circuit for the project. Tested it.
- Fully closed the UART chunk-receive protocol - sentinel acknowledgement.
- Started developing and implementing a host Python script.

**Evening**:
- Continued to developing and implementing the Python host script.

**Problems encountered:**
**1. Bench verification bug:**
Measured resistance across the button while unpowered - it gave a nonsensical 14.7kΩ reading (button open) instead of the expected open circuit (OL).
Why: 14.7 = 4.7 + 10 -> suspiciously exact for a coincidence.
The multimeter's own test current, pushed through the probes while unpowered, was finding an alternate path back through the 4.7kΩ pull-up and whatever else sits on that rail - measuring an unrelated combined circuit path instead of the switch actual open/closed state in isolation. Resistance measurement on a component still wired into a larger circuit is unreliable for exactly this reason.

Correct test: power the board, measure voltage (not resistance) directly across PB13's node and GND, in-circuit. Button open: 3.3V (pulled up cleanly, no path pulling down). Button pressed: 1.8mV (effectively zero, well within noise for shorted to GND).
This is the more faithful test overall, since it checks the exact electrical condition the firmware's GPIOB->IDR read actually depends on, under the real operating power condition, rather than an unpowered measurement vulnerable to picking up unrelated paths.

**What was done:**
**Python Host-Side Uploader Script (Part 1):**
Built from scratch.

*Stage 1 - serial port setup:*
Port /dev/ttyUSB0, baud 115200. First timeout value picked arbitrarily as 0.002s (matching the device's own 2ms inter-byte timeout), but that number answers a different question - the device's 2ms bounds the gap between bytes within a packet on the device side; the host's read() after sending a full packet has to wait through: full packet reception time, checksum computation, and - for a real chunk - a full flash_program() call (unlock, PSIZE set, 32-word write loop with per-word BSY poll).

Worked the actual budget from real numbers: 133 bytes at 115200 baud, 8-N-1 framing = 86.8 µs/byte * 133 = ~11.54ms for transmission alone. Flash programming: datasheet gives 16µs typ / 100µ max per ford at PSIZE = x32, 32 words = up to 3.2ms. Added 3ms generous margin for checksum/state-machine overhead, 1ms for the ACK byte to phisically clear the shift register and arrive. Total: ~20 ms, rounded up with 10ms safety margin -> 30ms final host-side per-chunk timeout.

Decided to hardcode this same 30ms for every chunk including the sentinel - reasoned that using the flash-inclusive timeout for the sentinel (which never calls flash_program) only means waiting a little longer than strictly necessary on that one path, a harmless direction to be wrong in.

*Step 2 - reading and chunking the .bin file:*
open("file.bin", "rb") + .read() to load the whole image into memory at once - no reason to stream incrementally given file size. Chunk count via ceiling-division using pure integer arithmetic: (len(content) + 128 - 1) // 128 - avoided math.ceil on a float division deliberately.

First chunking attempt used content.slice(...), which does not exist as a method on Python bytes objects - corrected to bracket-slice syntax content[i * 128 : i * 128 + 128]. 
Also caught a length-comparison bug: if new_chunk != 128 compares a bytes object to an int, which is always True regardless of actual length - needed len(new_chunk) != 128 instead. And the padding-count calculation was computing from len(content) % 128 (the whole file's remainder) rather than len(new_chunk) (the actual short slice in hand) - happened to produce the right number by coincidence on the true last chunk, but wasn't actually reasoning from the right variable. Fix: padding based on the actual slice length.

Final padding logic: last chunk, if short, gets += (128 - len(new_chunk)) * b'\xff' - padding with 0xFF is provably safe since erased flash is already all-1s (established earlier on the firstware side).

**Root cause at the register level:**
-

# 2026-09-07

**Morning:**
- Wrote a Linkedin post about the ORE/flash-erase discovery - why raw UART streaming is unsafe during flash erase (RXNE never cleared while flash is stalled -> Overrun Error -> protocol desync).
- Finished the jump sequence implementation.
- Started implementing the HardFault_Handler: grasped what should I do and why do I need to do it (theory part).

**Afternoon:**
- Implemented the HardFault_Handler.

**What was done:**
**1. HardFault Handler Implementation:**

*Naked entry stub:*
Assumed at first that ALL vector-table IRQ handlers had to avoid the naked attribute, based on a vague sense that vector table entries needed to be handled specifically. Traced through why that instinct was wrong: the actual deciding factor is not whether a function sits in the vector table, it is whether the handler needs raw pre-prologue access to hardware state and whether it needs a normal exception-return at all. USART2_IRQHandler is correctly not naked - it just reads SR/DR, does bookkeeping, and returns normally via the compiler's ordinary exception-return machinery. HardFault_Handler is different specifically because it needs to inspect the stacked frame before anything (including a compiled prologue) touches the stack, and because the bare-bones design never returns at all - it halts.

Wrote the naked stub: tests bit 2 of LR (EXC_RETURN) via tst lr, #4 and an ite eq conditional pair, selects MSP or PSP into r0 accordingly, tail-branches into a plain C function (HardFault_Handler_C) with that pointer as the argument.

*Capture destination:*
First attempt discarded everything - first pass at HardFault_Handler_C pulled all the right fields (faultStackedRegs[6] for PC, CFSR from its address) but cast every one of them to (void) and discarded them - nothing was actually written anywhere. Caught this by going back to the earlier conclusion that local variables have no guaranteed lifetime once sitting in the halt loop, especially at higher optimization levels - marking them volatile locally doesn't fix that, volatile only prevents the compiler eliminating a read/write, it does nothing to guarantee the storage location survives past the function's own frame.

Fix: built a proper global struct (static, not local) with fields for PC, R0-R3, R12, LR, xPSR, CFSR, HFSR, MMFAR, BFAR, plus explicit boolean validity flags (mmfar_valid, bfar_valid) and hfsr_forced, all marked volatile so the optimizer can never treat the writes as dead stores just because nothing in the C abstract machine reads them back.

**Problems encountered:**
**1. Register naming bug - MMAR vs MMFAR:**
Initially named a struct field MMAR, based on a table header in the reference doc.
Checked the actual descriptive paragraph in the same document and confirmed the correct name is MMFAR. Corrected the field.

**2. bkpt #0 hazard:**
Original halt-loop implementation included bkpt #0 before the infinite loop, intended to force a debugger break. Traced what BKPT actually does with no debugger attached (DHCSR's halting-debug-enable and monitor-debug-enable bits both clear, which is the normal state of any deployed board not connected to SWD): it generates a HardFault, not a silent no-op. Since this instruction would executre while already inside HardFault_Handler, at priority -1, with the original fault never having returned - this is exactly the "fault escalation with nowhere to go" condition that leads to Lockup, the same chain derived during the bootloader's SP-validation design. Removed bkpt #0 entirely - the bare while(1) already gives full debugger inspection capability (breakpoint on the loop line, or just halt manually) without ever risking Lockup on hardware with no debugger attached.

**Root cause at the register level:**
-

# 2026-09-06

**Morning:**
- Continued implementing and debugging the UART chunk-receive protocol in the bootloader project.
- Incorporated the retry logic and ACK/NACK to the host from the bootloader.

**Afternoon:**
- Finished implementing the UART chunk-receive protocol in the bootloader project.
- Developed (designed) the jump function (sequence).

**What was done:**
**Retry mechanism:**

**Bug 1:** 
On timeout, incremented a retry counter and reset the SysTick start time, but never reset chunk_state or payload_index back to the start of the packet. This meant the receiver just kept waiting for the exact same byte position it was already stuck on, up to 3 times, before aborting - not the "3 full chunk re-attempts" I'd actually designed.
Also: no NACK was ever sent on timeout, so the host had no signal a stall occurred.

Fix: added a did_retry_hit flag that, when set, resets chunk_state = UART_START_BYTE and payload_index = 0, sends a NACK, and loops back to wait for a completely fresh packet.

**Bug 2:**
After adding the did_retry_hit reset in UART_RECEPTION, the corresponding handling in UART_CHECKING set reception_state = UART_CHECKING (should have been UART_RECEPTION) before continuing - this meant the next outer_loop iteration landed straight back into UART_CHECKING, re-evaluating stale data from before the timeout, without ever going back to wait for a new byte. 

Fix: set reception_state = UART_RECEPTION in that branch instead.

**Bug 3:**
The checksum-corruption path (is_corrupted) had the identical missing-reset bug - set the flag, sent the NACK, but never reset reception_state back to UART_RECEPTION before continuing, causing it to loop forever re-checking the same stale packet and sending the same NACK repeatedly with no change to receive anything new.

**Bug 4:**
is_corrupted was a permament one-way flag, never cleared on a subsequent successful retry. If chunk 3 failed once, then succeeded on retry, and the transfer completed normally afterward, the function would still return UART_CORRUPTED at the end - even though the transfer actually succeeded. 
Same root pattern as the SER leftover bug in the flash routine: that was true once, read later as if still current.

Fix: cleared is_corrupted (via a corrupted_counter reset) the moment a chunk succeeds cleanly.

**Bug 5:**
Checksum failures had no bound at all - only the timeout path had a 3-strike budget. If the link was simply noisy and every retransmission of a chunk also failed its checksum, this would loop forever with no terminal state - an unbounded loop, exactly the "no bus hangs" criterion violation.

Fix: gave corruption its own independently bounded 3-strike counter, structurally mirroring the timeout retry logic.

**Bug 6:**
flash_program()'s FLASH_ERROR return was completely unhandled - no flag set, nothing. If a chunk's checksum passed but the actual flash write failed, and the transfer otherwise completed to the sentinel, the function would return UART_OK - the caller would believe the whole image was written correctly when one chunk had silently failed to program. 

Fix: added a dedicated error_counter with its own 3-strike bounded retry, sending UART_NACK_FLASH and looping back for a resend on transient failures, escalating to is_flash_error/break after 3 failures.

**Bug 7:**
Even after all retry paths were correctly bounded and reset, the flash-result success branch (FLASH_OK) never advanced reception_state back to UART_RECEPTION either. 
Trace: on a successful flash write, execution fell through to the bottom of UART_CHECKING with reception_state still stuck at UART_CHECKING - the next outer iteration would land straight back into UART_CHECKING and reprogram the exact same chunk at the exact same address indefinitely, never moving on to receive chunk 2. 

Fix: added the same reception_state = UART_RECEPTION; payload_index = 0; continue; pattern to the success branch as well.

**main() control flow - two-branch boot logic:**

*First pass:*
GPIO check happened, but the "stay in bootloader" branch was an empty else block - uart_chunk_receive_protocol() was never actually called anywhere in main(). Same for the SP-validation-failure case inside the jump branch - recognized in a comment but there was no code for it.

*What should happen in each branch:*
- For the "button held" branch: decided a successful reflash (UART_OK) should immediately attempt execute_user_application() - treating a successful flash as "try booting it now" rather than requiring a manual reset - for better user experience.
All four failure outcomes (overflow, retries, corrupted, flash-error) collapse to the same response: loop back and wait for another attempt, no special handling needed per-outcome, since uart_chunk_receive protocol() already re-erases the sector at its own entry point on every call regardless of why the previous attempt failed.

For the SP-validation-failure: initially unclear whether a second, separate handling was needed for UART_OK there. Traced the actual control flow and found it was already structurally correct without any extra code - logic_state is captured once before the loop and never changes, so on the branch where SP validation failed and uart_chunk_receive_protocol() got called, the next iteration of the outer while(1) naturally lands back in the same if(logic_state) branch and calls execute_user_application() again automatically. If the reflash succeeded, SP validation now passes and the jump happens; if it failed, SP validation fails again and the process repeats. No switch statement needed - the fixed value of logic_state across loop iterations gave the retry semantics for free.

The "button held" branch had no equivalent mechanism, since logic_state is permanently false in that branch and never triggers execute_user_application() on its own - required an explicit call added: 
if (uart_chunk_receive_protocol() == UART_OK) {
execute_user_application();
}

**Problems encountered:**
- **Bug: chunks_received 0ff-by-N across retries:**
Once every retry path was correctly resetting state and looping back to receive a fresh copy of the same chunk, a new bug surfaced: chunks_received was incremented unconditionally inside UART_END_BYTE - meaning a chunk that failed and got reset would increment the counter twice (once per attempt), even though only one of those attempts ever succeeded. This meant flash_program's address computation, which relies on chunks_received to determine which 128-byte slot in flash to write to, would drift forward by one slot for every retry anywhere in the transfer - silently overwriting the wrong flash addresses.

Fix: decremented chunks_received in every branch that causes a full chunk resend after the increment already happened - corrupted-checksum, flash-error, and overflow branches, all of which only execute after UART_END_BYTE had already run for that attempt.

One decrement was wrong, though: applying the same fix to the did_retry_hit (timeout) branch. Traced carefully: timeout fires from inside the byte-wait loop, which by construction always happens before UART_END_BYTE had run for that attempt - meaning the increment never happened in the first place for a timed-out attempt. Decrementing there would incorrectly subtract from a counter that was never incremented for that specific failure, causing the next successful chunk to land one slot too early, overwriting the previous chunk's already-correct data. Removed the decrement from that one branch specifically.

Applied the same "was an increment ever real for this attempt" check to the terminal is_retries about branch - concluded no decrement was needed there either, for two independent reasons: no increment ever happened (same structural reason as did_retry_hit, since is_retries also fires from inside the byte-wait loop before UART_END_BYTE), and the function is about to return and break out of the loop entirely regardless, so chunks_received's value is dead the instant the function returns.

- **Bug: 16 KB overflow boundary:**
Original check was all_payload_bytes >= 16384. 16384 bytes is exactly 128 full 128-byte chunks - the maximum legal image size that exactly fills Sector 2, a legitimate, correct-size transfer, not an overflow condition. The >= check would reject the 128th chunk - the one that exactly and correctly fills the sector - as an overflow error even though nothing illegal happened. Fixed to > 16384, which only trips on chunk 129 and beyond.

Decided that once overflow trips, the outer loop should break immediately (send UART_NACK_OVERFLOW, abort) rather than continuing to silently drain and discard incoming bytes - matches the same immediate-terminal treatment as retries-exhausted and corruption-exhausted, adn gives the host an actual signal instead of leaving it sending into silence.

- **The most significant late-stage bug - ACK missing on the success path:**
For a long stretch, the success path (checksum passes, flash_program succeeds) sent nothing back to the host at all. Given the entire protocol is built around the host blocking on a response before sending the next chunk, this meant a working transfer would just silently stall from the host's perspective after every single chunk. 

Fix: added the ACK send - but only after flash_program's actual return value confirms the write succeeded, not right after the checksum passes. Originally the ACK was being sent before flash_program ran at all, which meant the host could be told "success" while the flash write itself hadn't happened yet or has failed - checksum only proves the bytes arrived over UART intack, it says nothing about whether the write to flash succeeded.

*Sentinel ACK placement:*
Needed one final ACK after the sentinel packet arrives, so the host has positive proof the bootloader reached a clean end-of-transfer state (as opposed to trusting silently that everything worked). Placed the ACK inside UART_PAYLOAD_LEN, right where is_last gets set - not after the loop exits - because several other exit paths (retries exhausted, overflow, corruption exhausted) also cause the loop to end without the sentinel ever arriving, and an ACK placed after the loop would incorrectly fire on those paths too.

*Return type built:*
UART_ChunkReceive_ReturnTypes_t with UART_OK, UART_OVERFLOW_ABORT, UART_RETRIES_ABORT, UART_CORRUPTED, UART_FLASH_ERROR. Originally included a combined UART_OVERFLOW_RETRIES_ABORT state for when both overflow and retry-exhaustion could theoretically both be true - removed once overflow was changed to immediately break the loop, since after that change neither condition can survive to interact with the other in a later chunk.

**Root cause at the register level:**
-

# 2026-09-05

**Morning:**
- Learned about why does a bootloader exist and a usage of bootloaders in automotive industry.
- Finished developing (designing) the UART chunk-receive protocol.
- Designed the GPIO-pin check.

**Evening:**
- Implemented the GPIO-pin check.
- Mid-way implementing the UART chunk-receive protocol.

**Problems encountered:**
**1. Byte-to-word reconstruction:**
First attempt: `uint8_t array[4] = {payload[0..3]}; uint32_t word = *(uint32_t*)array;`

Two separate problems with this, both caught before writing it into the real function:
* Alignment isn't guaranteed by the language - uint8_t only requires 1-byte alignment, nothing forces a local array onto a 4-byte boundary, and Cortex-M4's unaligned-access behavior depends on the UNALIGN_TRP bit in SCB->CCR, which I had not checked.
* Strict-aliasing violation - reinterpreting a uint8_t[4] through a uint32t* is accessing memory through a pointer type that doesn't match its effective type. Undefined at the language level, independent of hardware, and exactly the calss of pointer conversion MISRA restricts.

Decided against memcpy() as well, on a personal consistency preference - I already do manual bit-shift reconstruction in other projects: word = payload[0] | (payload[1] << 8) | (payload[2] << 16) | (payload[3] << 24). Cortex-M4 is little-endian, so the byte that arrived first (payload[0]) naturally ends up as the LSB - no manual reordering needed, just correct shift positions.

**Bug: Address arithmetic bug:**
flash_program((FLASH_SECTOR2 * (32 * (chunks_received - 1))) + 1, ...) - multiplying an address by a chunk-derived number produces an arbitrary, meaningless value; addresses only make sense with offsets added, never multiplied.
Also noticed: 32 is the word count per chunk, not the byte count - using it directly as a byte offset would have silently computed an offset 4x too small, causing chunk 2 to overwrite chunk 1 instead of landing after it.

Fix: FLASH_SECTOR2 + 128 * (chunks_received - 1) - 128 bytes per chunk, added as an offset to the fixed base address, correctly landing each chunk sequentially.


**UART Chunk-Receive protocol design and implementation:**

*Packet layout:*
Start_byte(1) -> payload_len(1) -> payload[128] -> checksum (16-bit additive, big-endian: high byte, then low byte) -> end_byte (1). Total: 133 bytes per chunk.

Chose additive checksum over CRC deliberately - simpler arithmetic, acceptable blind spot (insensitive to byte reordering and certain paired bit-flips) for a short physical UART link with mostly single-bit/burst noise. Documented explicitly that this is a known limitation, not an oversight - CRC upgrade is legitimate future scope.

Payload_len kept as a meaningful field (not redundant with the fixed 128-byte wire size) - it tells the bootloader how many of the 128 bytes are real image data versus 0xFF padding on the final chunk. Padding with 0xFF up to a 4-byte multiple is provably safe: erased flash is all-1s, and writing 1-bits onto already-erased flash changes nothing electrically - confirmed against the manual's own note that changing bits from 1 -> 0 requires an erase, but writing 1 onto 1 requires nothing.

End_byte kept deliberately as a resync sanity check, and for future-proofing if payload size ever becomes dynamic instead of fixed.

*Sentinel packet:*
Payload_len = 0 signals end-of-transfer. Confirmed against my own state machine: UART_PAYLOAD_LEN case checks payload_len == 0, sets is_last = 1 immediately, never proceeds to read payload/checksum/end_byte for that packet - so the sentinel really is only 2 bytes on the wire (Start_byte + Payload_len = 0), not a full 133-byte packet with dummy filler.

*Reception mechanism:*
Polling loop, not interrupt-driven. Justified from the actual timing: the protocol is strictly half-duplex request/response (host sends chunk, waits, bootloader ACKs/NACKs, host sends next) - the bootloader has nothing else to do while waiting, so a blocking poll loop is simpler and just as correct as interrupts here.

*Inter-byte timeout derivation:*
Baud rate 115200, 8-N-1 framing = 10 bit-frames per byte = 86.8 µs per byte on the wire. Wanted roughly 5x margin over that (not too tight, not too loose) = ~434 µs.
But SysTick resolution is 1ms per tick, and a 1-tick countdown has a real hazard: if the start measurement lands one cycle before the tick boundary, the "1 tick" timeout can fire after almost no real time has elapsed at all. Minimum safe value is 2 ticks (2ms), guaranteeing at least 1 full tick of real margin regardless of phase alignment. 2ms against a required 434µs margin is actually a ~23x margin, not 5x - but the binding constraint became tick resolution, not the original margin target. No cost to this looseness in the happy path - timeout value only affects failure-detection latency, never throughput, since every real byte arrives well within 86.8µs and reloads the countdown long before it could expire.

Chose inter-byte timeout (resets on every byte arrival) over a whole-chunk fixed deadline - a whole-chunk deadline sized around the flash-timing budget would false-fail a slow-but-steady host (e.g. 128 bytes at 50ms gaps = 6.4s total, failing every chunk even on a healthy link). Inter-byte timeout only fires on a genuine stall, regardless of how slowly the rest of the chunk trickles in.

**GPIO Boot-Trigger:**

*Pin/resistor:*
PB13, chosen because it's unused by any other driver in the project. External pull-up used exclusively (consistent with never using internal PUPDR in any project) - originally planned 5kΩ, switched to 4.7kΩ once that's what was actually on hand (as I found out, 5kΩ is not even a standard resistor value). 4.7kΩ is the closer standard part anyway, and exact resistance does not actually matter for a clean digital logic read.

*Polarity decision:*
Button open -> pin reads 1 (via external pull-up to VDD) -> jump to application. Button held -> pin reads 0 (shorted to GND) -> stay in bootloader.
Chose this deliberately as the safer default: a dead battery, unpressed button, or normal power-up never accidentally strands the device in bootloader mode - deliberate action is required to enter the exceptional path, not the normal one.

*Debounce:*
Worked out the actual settle-time math rather than assuming a delay was needed. 
At 16MHz HSI, 1 cycle = 62.5ns. Worst case bounce settle time ~9ms = 144 000 cycles. But this number assumes the button press and MCU reset happen at the exact same instant, which they never do in practice - the user is holding the button down well before or during reset, contacts settle within the first few ms, and the user continues holding for hundreds of ms to seconds. By the time Reset_Handler finishes zeroing .bss, copying .data, and main() reaches the actual GPIO read (2-3 instructions), the button has already been in a steady settle state for far longer than the bounce window. No debounce delay needed - confirmed by tracing actual timescales rather than assuming a guard was required.

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

**Bug 1: sector number written to FLASH_CR without shifting:**
FLASH->CR |= sector_num; - cleared bits 6:3 for SNB right above this line, but then ORed sector_number straight in with no shift. A sector_num of 2 landed in bits 1:0, colliding with SER/MER bits instead of the actual SNB field.

Fix: pre-shift the sector values inside the enum itself instead of shifting at the call site - FLASH_SNB 2 = (2 << 3) etc. Bare |= now places the value correctly and the enum prevents anyone from passing an unshifted raw number by mistake.

**Bug 2: BSY not checked on function entry:**
Neither flash_program() nor flash_erase() checked BSY before touching FLASH_CR, only after. Traced through my actual call pattern: both functions poll BSY to 0 internally before returning, so entering either function fresh with BSY=1 isn't reachable as long as nothing else (no ISR) ever touches FLASH_CR concurrently. Confirmed the UART RX path never touches flash registers at all - single-writer architecture holds. BSY pre-check is legitimately dead code given this and was skipped.

**Bug 3: PSIZE never set inside flash_erase():**
flash_program() explicitly sets PSIZE = 10 (x32) every call. flash_erase() never touched PSIZE at all. FLASH_CR resets with PSIZE = 00 (x8) - it only becomes x32 once flash_program() has actually executed once. In my actual call order (erase Sector 2, then program it chunk by chunk), PSIZE was still 0 at the moment the very first erase set STRT. 16 KB sector erase at PSIZE = x8 is max 800ms - busts straight through the 500ms +  margin timeout I'd already derived for the host side.

Fix: added the PSIZE = 10 write inside flash_erase(), before STRT, so the 500ms budget (not 800ms) actually governs the timing.

**Bug 4: leftover SER bit not cleared after erase completed**
flash_program() correctly clears PG after the write loop. flash_erase() cleared STRT after BSY dropped but never cleared SER (bit 1) - the bit that activates sector-erase mode in the first place. After flash_erase() returns, SER is still 1 in FLASH_CR. The very next call is flash_program(), which sets PG=1 without SER ever being cleared.

Traced whether this is a live simultaneous-request hazard or just stale leftover state: the erase has already physically finished by the time the program call starts - SER=1 at that point is not a real "erase and program requested at the same time" situation, it's justa control register that was never correctly reset. PGSERR's actual trigger condition ("write access performed while control register not correctly configured") fits this exactly.

Fix: added FLASH->CR &= ~(1UL << 1U); to cear SER right after the STRT clear, before the function returns.

**Bug 5: error flags never checked after operations:**
Both flash_program() and flash_erase() polled BSY to 0 and returned - never read FLASH_SR for ony of the 5 unconditional error flags. Silent success/failure either way.

Resolved that error bits are guaranteed valid the instant BSY clears - no dependency on PG/STRT still being set - so the check belongs after PG/STRT are cleared, when no program or erase operation is still possible. Both functions now have typed return values (FLASH_OK/FLASH_ERROR) and call a shared flash_error_checking() function after clearing their respective control bits.

**Bug 6: flash_program's parameter shape mismatch**
flash_program(uint32_t * addresses, uint32_t * data, uint32_t len) assumed a caller would hand it a pre-built array of individual word addresses. In practice I only ever have one base address per chunk, with every subsequent word address entirely derivable (base, base + 4, base + 8 ...). There was no reason to force the caller to build an address array when the pattern is fully predictable.

Fix: changed signature to flash_program(uint32_t address_base, uint32_t data, uint32_t len), computing each word's target address inside the loop as address_base + (i * 4), case through volatile uint32_t to prevent the compiler from reordering or eliding the writes.

**Root cause at the register level:**
-

# 2026-09-03

**Morning:**
- Finished designing the bootloader.
- Implemented the FLASH register map struct, the address define macro and the useful Flash enums.
- Wrote a Linkedin post about the pre-work I have had before even touching the bootloader development.

**Evening:**
- Implemented the Program and Erase Flash functions and debugging them.

**What was done:**
**1. Bootloader design:**

*Partition finalized:*
Bootloader = Sector 0 + 1 (32 KB), application = sector 2 + starting at 0x08008000.

*Boot trigger:*
GPIO pin read at reset. RTC_BKPxR (backup registers, requiring DBP unlock in PWR_CR) deferred - unnecessary complexity for v1.

*Protocol and why raw streaming is unsafe:*
During flash erase or programming, instruction fetches from flash stall. Sector erase time - 16 KB sector max 500 ms. During that 500 ms window, the bootloader's own UART ISR code (sitting in flash) cannot be fetched. Incoming UART bytes arrive with nothing pulling them from DR. RXNE stays set. Next byte arrives before DR is read - RXNE still set when new data arrives in shift register. ORE (Overrun Error) asserts. The incoming byte is lost. Protocol desynchnorizes.

Fixed-chunk ACK/NACK protocol solves this perfectly: host sends one chunk, waits for ACK before the next. Host timeout must exceed max erase time + max program time with margin - a too-short host timeout causes retransmission into a deaf bootloader, ORE, desync.

*Flash unlock sequence:*
* Write KEY1 (0x45670123) and then KEY2 (0xCDEF89AB) to FLASH_KEYR.
* Check that LOCK bit (bit 31) of FLASH_CR reads 0 to confirm unlock. 
Any wrong key or wrong order re-locks and sets an error flag.

*Erase sequence:*
* Poll BSY (bit 16 FLASH_SR).
* Set SER (bit 1 FLASH_CR).
* Write SNB`[3:0]` (bits 6:3 FLASH_CR) with target sector number.
* Set STRT (bit 16 FLASH_CR).
* Poll BSY until clear.
* Check error flags.

*Program sequence:*
* Poll BSY.
* Set PG (bit 0 FLASH_CR).
* Set PSIZE = 10 (bit 9:8 FLASH_CR) = 32-bit parallelism - legal at VDD = 3.3V (minimum 2.7V).
* Write 32-bit word to target flash address.
* Poll BSY.
* Check error flags.
* Repeat for each word in chunk.
* After last word confirmed - clear PG.
* PG is held across the entire word-write loop - no window where a write happens with PG deasserted.

*5 unconditional bits of FLASH_SR error flags:*
WRPERR (bit 4), PGAERR (bit 5), PGPERR (bit 6), PGSERR (bit 7), RDERR (bit 8). All set regardless of interrupt enable state.

OPERR (bit 1) and EOP (bit 0) are gated by ERRIE and EOPIE respectively - excluded from the error-check list.

*Application validation before jump:*
SP range (0x20000000 - 0x2001FFFF) + alignment (bits 2:0 = 000) only. No image CRC.

*Jump sequence:* 
Validate SP -> MSR MSP word0 -> write SCB_VTOR to 0x08008000 ->branch to word1 (Reset_Handler address from vector table[1]).

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

**What was done:**
**1. Bare-Bones HardFault Handler:**

*What was studied:*
On exception entry, hardware auto-stacks 8 words to whichever stack was active: R0, R1, R2, R3, R12, LR, PC, xPSR in fixed order. The stacked PC is the faulting instruction address - the most useful diagnostic piece.

*Why a plain C function is not enough:*
A C function generates a compiler-produced prologue before the first line of user code runs - pushes R4-R11, may adjust SP for locals. By the time the C code's first line executes, SP has moved past the hardware-stacked frame. No register or variable in C scope automatically equals "the hardware-stacked frame".

*Correct architecture:*
Naked asm stub (GCC `__attribute__((naked))`) as the actual HardFault_Handler symbol. Naked instructs the compiler to emit zero prologue/epilogue - the first instruction executes with SP exactly where hardware left it.
The naked function must be written entirely in basic inline assembly. From there, a tail-call into a real C function passes the resolved frame pointer as an argument - separating raw hardware context capture (asm, environment fully controlled) from complex diagnostic C logic.

*MSP vs PSP resolution:*
On exception entry, LR is loaded with EXC_RETURN. Bit 2: 0 = exception taken from MSP, 1 = exception taken from PSP. Currently (no RTOS) only MSP exists - but the bit-2 check is one TST/branch in asm already being written by hand. 
Skipping it hardcodes it to "always MSP" - the moment FreeRTOS gives tasks their own PSP and one hits a fault, the handler reads the wrong stack frame silently. No fault, no indication, just wrong PC/LR/xPSR printed with no attribution trail. 

*Fault status registers:*
* CFSR at 0xE000ED28: MMFSR[7:0], BFSR[15:8], UFSR[31:16]. Some bits are write-1-to-clear - read and save immediately on entry.
* HFSR at 0xE000ED2C: bit 30 FORCED - set when a configurable-priority fault (BusFault, MemManage, etc) escalated to HardFault because it was disabled or lower priority. If FORCED = 1, CFSR must be read for the real cause - HFSR alone won't tell you.
* MMFAR at 0xE000ED34 and BFAR at 0xE000ED38: faulting data address. Only valid if MMARVALID/BFARVALID in CFSR is set.

*No interrupt masking needed in the handler:*
HardFault sits at priority -1 - only NMI (-2) and Reset can preempt it. Every configurable interrupt, including SysTick, cannot fire while inside HardFault_Handler. The captured data cannot be clobbered by another interrupt.

*Capture destination - fixed global struct:*
We cannot leave the frame pointer in a register and let GDB walk the stack if we are using -O2 compiler optimization: it produces liveness analysis, register allocation, and dead code elimination - all of this gives no guarantee the pointer survives in a predictable register once it is no longer needed for computation - especially once sitting in a halt loop where the compiler may see it as dead.
The solution for this is a named global struct, written once by the C handler before entering the halt loop. It is readable by GDB by fixed address regardless of optimization lever or of debug symbol resolution.

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

**What was done:**
**1. SCB_VTOR - Vector Table Relocation**
*What was studied:*

SCB_VTOR lives at 0xE000ED08 (SCB base 0xE000ED00 + offset 0x08), reset value 0x00000000. TBLOFF field is bits [29:9] on STM32F411 - this is the vendor-specific value from PM0214, not the generic ARMv7-M doc which gives [31:7]. The difference exists because the F411 has ~98 total vector entries (82 external IRQs + 16 core exceptions), rounded up to 128 words = 512 bytes = 0x200 minimum alignment, which forces bits [8:0] to reserved-zero. The generic doc gives the formula; PM0214 gives the already-computed device-specific answer.

On exception entry, the NVIC performs a direct hardware memory read:`handler_address = *(VTOR + 4 * exception_number)`. No software dispatch, no checking - pure fixed-formula fetch. If VTOR points somewhere wrong, the CPU fetches garbage and jumps there with interrupts live.

*Without a bootloader - no need to write VTOR explicitly:*
At reset VTOR = 0x00000000. The boot alias (controller by BOOT pins) maps flash at 0x08000000 to also appear at 0x00000000 bit for bit. Word 0x00000000 = word 0x08000000 (`_estack`). Word 0x00000004 = word 0x08000004 (Reset_Handler). All subsequent NVIC lookups at VTOR + IRQn * 4 correctly resolve to the .isr_vector in flash. No explicit VTOR write needed - the alias makes the reset value functionally correct.

*With a bootloader - VTOR becomes load-bearing:*
If the bootloader lives at 0x08000000 and the application at 0x08008000, VTOR is still 0x00000000 (pointing at the bootloader's vector table via the alias) when the application starts running. If the application enables any interrupt without first writing VTOR to 0x08008000, the NVIC fetches the vector from the bootloader's table - not the application's.

Two failure modes depending on what the bootloader put in that slot:
* Bootloader has a Default_Handler (infinite loop `b Infinite_Loop`) for that IRQ: the application's interrupt flag is never cleared, the line stays asserted, the CPU re-enters the bootloader's Default_Handler endlessly. Main loop freezes, no HardFault raised, completely silent.
* Bootloader has a real handler for the same peripheral (e.g. USARTS for its own firmware update protocol): the CPU jumps to valid, correctly-aligned Thumb code - no fault at all. But it executes in the wrong context: reads DR/RDR (clearing RXNE the application was waiting on), may write bytes to the TX line with bootloader framing, may reprogram baud rate or parity bits, may corrupt application RAM if the linked maps overlap. No fault flag, no wrong PC visible in a debugger, just silent state corruption that manifests randomly elsewhere.

*Alignment check of 0x08008000:*
0x08008000 = 0b1000000000000100000000000000 ... bits [8:0] = 00000000. All zero - satisfies the reserved-zero requirement. Legal to load into TBLOFF.

*Lesson learned:*
VTOR much be written in the application's early startup, before any interrupt is unmasked, to the application's own vector table base. The ordering constraint: validate the application image first (while nothing is committed), then write VTOR, then unmask interrupts. If anything goes wrong after VTOR is written, reverting it while running on a potentially corrupted or mismatched stack is not safe - the window between the VTOR write and a correctly-running application is one where the system is in a partially-committed state.

**2. Flash Sector Layout - STM32F411 Partition Decision:**
*What was studied:*

Asymmetric sector layout:
* Sectors 0-3: 16 KB
* Sector 4: 64 KB
* Sectrs 5-7: 128 KB each

The layout is intentionally asymmetric - small sectors at the bottom give fine-grained boundary options for bootloader/config partitions. Flash erase is issued via FLASH_CR with SNB[3:0] selecting the sector. There is no sub-sector erase - minimum erase granularity is one full sector. This, not VTOR's 512-byte alignment, is the binding constraint on partition boundaries.

*Partition decision:*
* Bootloader: Sector 0 + Sector 1 = 32 KB. 32 KB gives room for a flash-write driver, CRC/checksum verification, and a minimal UART comms protocol without being cramped.
* Application: Sector 2 onward, starting at 0x08008000.

Both 0x08004000 and 0x08008000 land exactly on sector boundaries - neither is mid-sector. VTOR alignment (512 bytes, bits [8:0] = 0) is looser than what flash geometry already forces - it drops out as a non-issue.

*Self-update decision:*
Self-update capability (bootloader rewriting its own flash sector while running from it) is out of scope for v1. The bootloader is flash-once-via-SWD.
Consequence: no RAM-resident erase/program routine needed. If the bootloader ever issued a sector erase against its own sector while executing from it, the CPU would be fetching  instructions from flash mid-erase - not "risky", architecturally undefined. That's the same structural reason .data exists: code that must remain stable while flash is unstable must not be in flash at the moment. Deferred with explicit TODO in README.

**3. Stack Pointer Validation Before Application Jump:**
*What was studied:*

A bootloader jumping to an application is not a hardware reset. The CPU is already running. The hardware's automatic word-0/word-1 load (MSP from vector table[0], PC from vector table[1]) happened once at reset for the bootloader - it does not happen again. The bootloader must manually replicate that behavior: read the application's vector table word 0, load it into MSP explicitly (MSR MSP instruction), then branch to word 1 (Reset_Handler address).

*Why validate word 0 before trusting it:*

Word 0 is just data sitting in flash at the application's sector base. If the application was never flashed, the sector is is erased state - that means every bit 1, value 0xFFFFFFFF. If flashing was interrupted or used the wrong offset, it could be anything. The valid range for a legitimate `_estack` on this part: SRAM1 spans for 128 KB from 0x20000000 to 0x2001FFFF. Any value outside that window is invalid.

*Failure mode if the validation is skipped:*

MSR MSP with word0 = 0xFFFFFFFF: this is a core-register write, not a bus transaction. MSP is now 0xFFFFFFFF.

Branch to word1 (also 0xFFFFFFFF on an erased sector): CPU attempts instruction fetch from 0xFFFFFFFF - but no memory mapped there. Bus matrix returns a BusFault. BusFault is not separately enabled, so it escalates directly to HardFault.

HardFault entry on the other hand attempts auto-stacking: 8 registers pushed to MSP = 0xFFFFFFFF. Stack write targets 0xFFFFFFFF - 0x20 = 0xFFFFFFDR which is also unmapped. A fault during the stacking phase of another fault's entry with no valid stack = Lockup state. CPU halts instruction execution entirely. Only external reset or debugger intervention can recover it. There is no watchdog recovery, no software recovery.

Lockup cannot be classified as a hang - the difference between hangs and the Lockup state is that hangs are watchdog-recoverable, the Lockup is not. The "no bus hangs" done bach is not met without introducing the check.

*The check:*

* Range: word0 must fall within 0x20000000 - 0x2001FFFF
* Alignment: bits [2:0] must be 000 (8-byte alignment - matches what a standard ARM toolchain's `_estack` alsways produces, but it is stricter than the 4-byte architectural floor).

*Ordering:*

Validate word0 against SRAM range and alignment using a general-purpose register (MSP untouched, VTOR untouched). If check fails, then return false or abort - bootloader remains fully operational, recovery path (wait for new binary over UART, safe mode) available. Only after the check passes: MSR MSP word0 -> then write VTOR -> only then branch to word1.

The window between MSR MSP and a successful branch into working app code is a window where the CPU cannot survive any exception - any stack push of any kind during that window produces the Lockup state. Validation must be strictly before the instruction.

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
 
