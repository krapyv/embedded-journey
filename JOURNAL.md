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
 
