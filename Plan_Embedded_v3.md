# Plan v3: Junior Automotive Embedded (Poland, then Germany)

Revision of Plan v2. Same 33-week skeleton, with the pipeline time budgeted, interview prep moved earlier, a review schedule added, and the Week 14 flash/erase design corrected.
Daily schedule: Mon-Sat, 4.5 h (about 4 h plan + 30 min German in the evening). Sunday: complete rest.

**Start:** Mon 19 Oct 2026 (Week 1)
**Length:** 33 weeks, ending about 6 June 2027
**Start applying:** Week 12 (Mon 4 Jan 2027)
**All MUST items finished:** Week 20 (about 7 March 2027)

Priority legend
- ⭐⭐⭐ MUST: shows up in most junior postings or interviews. Never cut.
- ⭐⭐ IMPORTANT: clearly helps. Cut only if you are more than 3 weeks behind, or if interviews/take-homes need the time.
- ⭐ NICE: first to cut.

## What changed from v2

1. From Week 12, about 6 h/week is reserved for the application pipeline (tailoring, tracking, interviews, take-homes). Interviews and take-homes preempt any ⭐⭐ item. Weeks 21-27 are hedges, not commitments.
2. Interview drills are a standing slot from Week 12, not a Week 28 event. Week 28 is consolidation.
3. A review schedule (below) is now part of the plan. Budget: 8-10 h total.
4. Cheap visible signals before applying: build-only CI, README polish, AUTOSAR vocabulary (Week 8), CV v2 finished in Week 11.
5. S32K144EVB purchase has a decision gate at Week 16.
6. Week 14 corrected: the erase/NRC 0x78 order, erase span from the 0x34 request, and a bootloader size check.
7. Week 1 CAN frame design must fit counter + CRC.
8. Small adds: UDS 0x28 / 0x85 / suppress-positive-response bit, TRACE32 awareness, honest MISRA wording.
9. Post-plan list trimmed to what is actually in scope.

---

## Where you are (checked against your own checkmarks)

Done: Phases 0-3 (bare-metal STM32F411, GPIO, SysTick, UART+DMA, timers, ADC, I2C, SPI, MCP2515 CAN, low power, bootloader, CAN sensor node), Phase 4 weeks 20-26 (ring buffer, queue, stack/list, search/sort, CRC, PID).

## Timeline at a glance

| Weeks | Dates (start of week) | Block | Priority |
|---|---|---|---|
| 1 | 19 Oct | Close Phase 4 (filters + integration), diagnostic, posting survey | ⭐⭐ |
| 2-7 | 26 Oct - 30 Nov | FreeRTOS + LIN | ⭐⭐⭐ |
| 8 | 7 Dec | HAL layering + visible signals (CI, READMEs, AUTOSAR vocabulary) | ⭐⭐⭐ |
| 9 | 14 Dec | ISO-TP | ⭐⭐⭐ |
| 10-11 | 21 Dec - 28 Dec | UDS server, CV v2 finished in Week 11 | ⭐⭐⭐ |
| 12 | 4 Jan | OBD-II (**start applying**) | ⭐⭐ |
| 13 | 11 Jan | CAN deep dive: DBC, tools, CAN FD concepts | ⭐⭐⭐ |
| 14-15 | 18 Jan - 25 Jan | **UDS bootloader (flagship)** | ⭐⭐⭐ |
| 16-17 | 1 Feb - 8 Feb | MISRA C, static analysis, unit tests, CI. **Board decision gate (Week 16)** | ⭐⭐⭐ |
| 18-19 | 15 Feb - 22 Feb | AUTOSAR Classic concepts + mini stack | ⭐⭐⭐ / ⭐⭐ |
| 20 | 1 Mar | Safety + ASPICE concepts | ⭐⭐⭐ |
| 21-23 | 8 Mar - 22 Mar | NXP S32K144 (only if the gate says yes) | ⭐⭐ (hedge) |
| 24-27 | 29 Mar - 19 Apr | C++ for embedded (compressed) | ⭐⭐ (hedge) |
| 28-30 | 26 Apr - 10 May | Interview consolidation, mocks, offers | ⭐⭐⭐ |
| 31-33 | 17 May - 31 May | Buffer + specialization | ⭐ |

Weeks 10-11 fall over Christmas. If you slow down there, the end buffer absorbs it.

## Weekly routine (all phases)

- Saturday = 4-hour project block. Finish with a commit, a README, and a photo or log capture.
- German: 30 min every evening. Target A2 by Week 30. Polish sites of German companies are a sensible bridge before a Germany move.
- **From Week 12: about 6 h/week for the pipeline.** 5-10 tailored applications, tracked in a spreadsheet (company, role, date, stage, requirements they listed), plus interviews and take-homes. Plan content drops to about 18 h/week. If an interview or take-home lands in a week, it preempts any ⭐⭐ item.
- **From Week 12: standing interview-drill slot.** 20 min on weekday evenings plus the last hour of Saturday. Topics come from your failed-question log and the review schedule.
- Keep a "failed interview questions" log. Whatever you failed becomes next week's evening theory.
- **Week 1: job-posting survey, split across two evenings** (about 40 min each, aim for 30 postings, not 40 in one sitting). Sources: Poland (Pracuj.pl, LinkedIn, Bosch/Aptiv/Continental/GlobalLogic career pages), Germany (StepStone, LinkedIn). Tally requirements. If "C++" appears in the majority, move Weeks 24-27 up to right after Week 17. If "AUTOSAR" dominates, keep Weeks 18-19 where they are. Also note which MCU families (S32K, AURIX, RH850) appear, for the Week 16 gate.

---

## Review schedule (8-10 h total, not 2-3)

Method for every topic: (1) explain it out loud in 2 minutes without notes, (2) draw the timing diagram or frame layout from memory, (3) answer the questions below. Pass: move on. Stall: reread only that part, then log it. Use your own logic analyzer captures as study material.

**Week 1, evenings (diagnostic, about 1.5 h total):** run the three-step method on every topic below once, including the interrupt/C-memory block and the C-traps block, so you know your gaps in the first week. After that: 20 min at the start of an evening block on whichever topic failed (Weeks 2-8), and re-drill interrupts/C memory in Weeks 9-11.

Priority order (CAN, C memory/interrupts and your own projects are co-equal at the top):

1. **CAN.** Why does the lowest ID win without destroying the frame? What do SJW and the sample point do? Why does bit rate depend on bus length? What are TEC/REC thresholds, and how does bus-off recover? Why 120 Ω at both ends? What do filters and masks do, and how does loopback differ from listen-only?
2. **Interrupts, concurrency, C memory.** What does the NVIC do with priority and pending flags? Why isn't `volatile` atomic? When can a single-producer/single-consumer ring buffer work without a lock? Walk through reset handler, `.data` copy, `.bss` zeroing, vector table, linker sections. Stack vs heap, struct padding, endianness.
3. **C language traps** (asked live). Integer promotion and signed overflow. What `volatile`, `const`, `static` each guarantee. Alignment and padding. Classic macro pitfalls. Whiteboard: set/clear/toggle a field, count bits, swap endianness.
4. **Your bootloader.** Boot sequence (VTOR, MSP, reset handler). Flash erase granularity. What happens if power dies mid-update. How do you validate the image?
5. **I2C.** Why open-drain and pull-ups, and what does bus capacitance do to them? START/STOP/repeated START, ACK vs NACK. Clock stretching and bus-hang recovery. Same-address conflict.
6. **SPI.** How do you pick CPOL/CPHA from a datasheet timing diagram? Why does a read need dummy bytes? SPI vs I2C trade-offs.
7. **UART.** Baud rate error sources, framing/parity/overrun errors, break condition (relevant to LIN).
8. **Peripherals as a group.** What sets PWM frequency vs duty? ADC sampling time vs resolution. What DMA removes from the CPU and what it doesn't. Independent vs window watchdog.
9. **RTOS basics** (review in Week 7). Priority inversion, mutex vs semaphore, which APIs are ISR-safe.
10. **Cortex-M exception entry.** What does hardware stack on entry? What does EXC_RETURN select? (You wrote a HardFault dump, so use that as your source.)

Demoted to one sentence each, do not drill: UART oversampling and flow control, ADC aliasing, NVIC tail-chaining detail, classic CAN vs CAN FD (one-minute answer).

Done bar: you pass the self-test on all topics, can explain your CAN node and bootloader end to end, and have a short log of stalled questions.

---

## Phase 4 (finish): Filters + Integration (Week 1) ⭐⭐

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 1 | Digital filters + integration | EMA, moving average, median filter on BMP280/ADC data | Filter trade-offs (lag vs noise); diagnostic and posting survey (see above) | Add filtering + an alive counter + CRC-8 to your CAN sensor node frames |

Design constraint (do this on paper first): your current frame is 8 bytes full with two 4-byte values, so there is no room for a counter or CRC. Choose scaled int16/uint16 signals, define scale and offset for each, and place the alive counter and CRC-8 in the remaining bytes. Write the layout as a table. This is the same thinking as the Week 13 DBC and the idea behind AUTOSAR E2E (check the exact profile details yourself when you get there).

Done bar: the CAN sensor node broadcasts filtered, counter- and CRC-protected values; you have a spreadsheet of requirements from the postings; the diagnostic is done and logged.

---

## Phase 5: FreeRTOS + LIN (Weeks 2-7) ⭐⭐⭐

Use the FreeRTOS kernel source directly on your STM32F411 (no HAL needed, your drivers stay).

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 2 | Tasks, scheduling, tick ⭐⭐⭐ | Port FreeRTOS to F411, create 3 tasks, observe preemption and priorities | RTOS concepts, context switch on Cortex-M (PendSV, SVC) | Multi-blink + UART logger task |
| 3 | Queues, semaphores, priority inversion ⭐⭐⭐ | Sensor task -> queue -> CAN TX task | Priority inversion, priority inheritance | Sensor reader + CAN sender |
| 4 | Mutexes, event groups, task notifications ⭐⭐⭐ | Printf mutex; CAN RX ISR -> task via notification (deferred interrupt processing) | Deadlock, ISR-safe APIs (FromISR) | CAN RX task pattern (reused by UDS later) |
| 5 | Software timers, stack overflow detection, runtime stats ⭐⭐ | Measure stack high-water marks, enable overflow hook | Watchdog design | Task monitor + task watchdog |
| 6 | **LIN (software + two-UART bench)** ⭐⭐⭐ | LIN frame: break, sync, PID with parity, data, enhanced checksum; schedule table; master task + slave response | LIN vs CAN (single-wire, master/slave, 20 kbit/s, where used: doors, seats, mirrors) | LIN master + slave between two UARTs of the same board |
| 7 | Integration | Multi-task CAN sensor hub: I2C sensor task, CAN TX/RX tasks, UART log, watchdog monitor | Documentation; RTOS basics review; **CV v1** | **Portfolio: rtos-can-sensor-hub-c** |

**Answer on paper before Week 2 (FreeRTOS port):**
- What does `configMAX_SYSCALL_INTERRUPT_PRIORITY` constrain? What does that mean for the priorities your existing EXTI, USART and DMA ISRs already use?
- The F411 implements 4 priority bits. Where do they sit in the byte you write, and how does the NVIC priority grouping you chose in Phase 3 interact with that?

**Answer on paper before Week 6 (LIN, two UARTs, one wire):**
- Both TX pins can't be push-pull on one line. What output configuration makes the line behave like a real LIN bus?
- Each UART will receive its own transmission. How does your slave logic avoid treating its own echo as a master header?

LIN note: the real transceiver bus test happens in Week 22 on the S32K144EVB if you buy it. Otherwise a ~5 EUR LIN transceiver breakout does the same job.

Done bar (Phase 5): 4+ tasks, no stack overflows, no priority inversion in a deliberate test, CAN traffic stable for 10 minutes.

---

## Phase 6: HAL layering + visible signals (Week 8) ⭐⭐⭐

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 8 | Layered architecture, callbacks, service tables | Driver interface as function-pointer table (GPIO/CAN/UART); command table for a UDS-style dispatcher | Layers: driver -> ECU abstraction -> services -> application (the AUTOSAR layering idea); one evening on AUTOSAR layer vocabulary (MCAL, ECU abstraction, services, RTE, SWCs) | Refactor CAN node into 4 layers; same app code, swapped driver |

Also this week (cheap signals your first applications will link to, because the flagship does not exist until Week 15):
- Add a build-only GitHub Actions workflow with `arm-none-eabi-gcc` to your existing repos (about 1 h). Tests join it in Week 17.
- Polish the READMEs of the bootloader and CAN node repos: architecture diagram, memory map, how to build, a photo or log capture.

This week prepares Weeks 9-19 directly.

---

## Phase 7: Automotive Protocols in C (Weeks 9-15) ⭐⭐⭐ (the core of your junior profile)

### Week 9: ISO-TP (ISO 15765-2) ⭐⭐⭐

UDS cannot work without it: it carries messages longer than 8 bytes over CAN.

| Day | Morning (Coding) | Evening (Theory) |
|---|---|---|
| Mon | Frame formats: Single Frame, First Frame, Consecutive Frame, Flow Control; PCI bytes | Draw sender and receiver state diagrams |
| Tue | Implement Single Frame TX/RX (up to 7 bytes) | Test vectors, plain asserts (Unity comes in Week 17) |
| Wed | Multi-frame TX: First Frame + Consecutive Frames, sequence number wrap (0..15) | Flow control: block size, STmin |
| Thu | Multi-frame RX: reassembly, send Flow Control, timeouts (N_As, N_Bs, N_Cr) | Error cases: wrong sequence number, overflow, timeout |
| Fri | Test against `python-can` + `can-isotp` on PC through your CANable | Compare traces in SavvyCAN or Wireshark |
| Sat | 4-hour: ISO-TP library in C (static buffers, no malloc, callback-based send) | Document |

Done bar: send and receive a 100+ byte message in both directions between STM32 and PC, with flow control, no lost frames.

### Weeks 10-11: UDS Server ⭐⭐⭐

Use `python-udsoncan` on the PC as the tester. Your STM32 is the ECU. Build on Week 4's CAN RX task and Week 8's service table. Re-drill interrupts/C memory in these weeks (review schedule).

Week 10: Sessions + core services

| Day | Morning (Coding) | Evening (Theory) |
|---|---|---|
| Mon | UDS frame parsing on top of ISO-TP; service dispatcher table | Request/response format, SID + 0x40 positive response |
| Tue | 0x7F negative responses; NRCs 0x11, 0x12, 0x13, 0x31 | NRC catalogue; when each one is correct |
| Wed | 0x10 DiagnosticSessionControl (default / extended / programming) | Session state machine, S3 timeout (SysTick) |
| Thu | 0x3E TesterPresent; S3 timer reset | Timing: P2, P2* |
| Fri | 0x22 ReadDataByIdentifier, 0x2E WriteDataByIdentifier (DIDs stored in RAM/flash) | DID design; what needs which session |
| Sat | 4-hour: 0x11 ECUReset + integration test script in Python | Document the supported-services table |

Week 11: Security + diagnostics + CV

| Day | Morning (Coding) | Evening (Theory) |
|---|---|---|
| Mon | 0x27 SecurityAccess: seed generation, key check | Seed/key concept, attempt counter, delay after failures |
| Tue | Lock/unlock state tied to sessions; NRC 0x33, 0x35, 0x36, 0x37 | Why unlock status resets on session change |
| Wed | DTC storage (RAM first): status byte, simple debounce | DTC concepts: confirmed/pending, freeze frame (concept only) |
| Thu | 0x19 ReadDTCInformation (a couple of sub-functions), 0x14 ClearDiagnosticInformation | DTC lifecycle. Last 30 min: CV v2 draft |
| Fri | 0x31 RoutineControl; NRC 0x78 "response pending" | Long-running services. Last 30 min: CV v2 final + LinkedIn |
| Sat | 4-hour: full pytest suite against the ECU (`udsoncan` + `python-can`) | **Portfolio: uds-server-c + iso-tp-c** |

CV v2 is finished before Week 12, so the first Saturday of applying is not also CV day.

Done bar: a Python script performs a session change, unlocks security, reads/writes a DID, reads DTCs, and receives correct NRCs for bad requests, all without touching the firmware between steps.

### Week 12: OBD-II ⭐⭐ + START APPLYING

| Day | Morning (Coding) | Evening (Theory) |
|---|---|---|
| Mon | OBD-II request/response frames, 0x7DF functional ID, 0x7E8 response | OBD-II vs UDS (legislated emissions diagnostics vs OEM diagnostics) |
| Tue | Service 0x01: supported PIDs, 0x0C RPM, 0x0D speed, 0x05 coolant temp | Scaling formulas |
| Wed | Service 0x03/0x04 (DTCs, clear), 0x09 (VIN) | DTC format |
| Thu | Simulate an ECU: potentiometer/ADC -> RPM PID | Interview-drill slot starts |
| Fri | Read it with a PC tool (`python-can`, SavvyCAN) | J1939 awareness (reading only, trucks) |
| Sat | 4-hour: OBD-II simulator polish + README | **Send first batch of applications** (CV v2 is already done) |

### Week 13: CAN deep dive + tools ⭐⭐⭐

| Day | Morning | Evening |
|---|---|---|
| Mon | Write a DBC by hand for your node (scaling, offset) | DBC format; Intel vs Motorola byte order |
| Tue | Signal pack/unpack in C (watch big-endian bit numbering) | Bit-level layout practice |
| Wed | Generate C code with `cantools` and use it on the STM32 | Code generation in automotive (config-driven) |
| Thu | `python-can` + SavvyCAN/Wireshark: log, decode with your DBC | Bus analysis workflow |
| Fri | CAN recap: arbitration, bit stuffing, error counters, bus-off | CAN FD concepts: larger payload (up to 64 bytes), bit-rate switch, DLC mapping. Your MCP2515 cannot do CAN FD |
| Sat | 4-hour: DBC + decoder + restbus (simulated other ECUs) with `python-can` | CANoe/CANalyzer: awareness only |

### Weeks 14-15: UDS Bootloader (FLAGSHIP) ⭐⭐⭐

You already have a UART bootloader. This extends it so it reflashes over CAN with ISO-TP + UDS, exactly like production ECUs. **This is the most valuable project you can show.**

**Answer on paper before Week 14 (erase timing):**
- While the CPU is stalled on flash fetch, what does the MCP2515 still do by itself? How many frames can it hold?
- Which ISO-TP or UDS mechanism keeps the tester silent for that window?
- Order of events: when exactly must NRC 0x78 be sent relative to starting the erase, and why?

Week 14: Protocol + flash

| Day | Morning (Coding) | Evening (Theory) |
|---|---|---|
| Mon | Bootloader receives UDS over ISO-TP (reuse Weeks 9-11 code). **First, check bootloader size with `arm-none-eabi-size`**: ISO-TP, UDS, MCP2515 SPI, CRC and SecurityAccess may not fit in 16 KB; if not, decide the new app start address before anything else | Boot flow: reset -> validity check -> jump or stay |
| Tue | Application header: magic, length, CRC32; validity check before jump | Why check validity, what happens with a half-flashed app |
| Wed | 0x10 programming session, 0x27 SecurityAccess | Who is allowed to flash. UDS awareness: 0x28 CommunicationControl, 0x85 ControlDTCSetting, and the suppressPositiveResponse bit (implement it for 0x3E) |
| Thu | 0x34 RequestDownload first: validate address and length, NRC 0x31 on a bad range. Then the 0x31 erase routine: the erase span comes from the 0x34 address and length (a 20+ KB app spans several sectors). **Send NRC 0x78 before the erase starts.** The CPU stalls on flash fetch during the erase on a single-bank F411 | Flash erase stalls code fetched from the same bank; know the consequence for CAN RX |
| Fri | 0x36 TransferData with block sequence counter | Max block length, flow control interaction |
| Sat | 4-hour: 0x37 RequestTransferExit, 0x31 CRC check routine, 0x11 reset into app | Document the programming sequence as a diagram |

Week 15: Robustness + demo

| Day | Morning | Evening |
|---|---|---|
| Mon | Python flasher script with `udsoncan` (HEX/BIN -> UDS blocks) | |
| Tue | Failure tests: wrong sequence counter, bad CRC, wrong session, locked state | Expected NRC for each case |
| Wed | Power loss in the middle of a transfer: bootloader must stay recoverable | Atomic update ideas (image flag written last) |
| Thu | Unit-test the parsing and CRC on PC (preparation for Week 17) | |
| Fri | README: architecture diagram, memory map, sequence diagram | |
| Sat | 4-hour: demo video + GitHub release | **Portfolio: uds-bootloader-c** |

Done bar: flash a 20+ KB application over CAN from a Python script with no UART involved; interrupt power halfway and confirm the bootloader still comes up; the app boots after a good flash.

---

## Phase 8: Quality Tooling (Weeks 16-17) ⭐⭐⭐

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 16 | MISRA C:2012 + static analysis | Build with `-Wall -Wextra -Wconversion`; run cppcheck (MISRA addon) and clang-tidy on your bootloader + UDS code; fix real violations, document deviations | MISRA categories (mandatory, required, advisory), deviation procedure, why rules exist. **Board decision gate (see Phase 10)** | MISRA report for uds-bootloader-c |
| 17 | Unit testing + CI + workflow | Unity (+ CMock or fff) tests for ISO-TP, UDS dispatcher, CRC, ring buffer on the PC; gcov coverage (statement + branch) | Coverage levels, MC/DC (concept); Git branching, pull requests and review, Jira/Agile in one evening | GitHub Actions: build + tests + cppcheck on every push (extends the Week 8 build-only workflow) |

Done bar: a green CI badge on your main repo; tests cover ISO-TP and UDS parsing; your CV can honestly say "applied MISRA C:2012 guidelines using static analysis, Unity unit tests, CI". The free cppcheck addon covers only a subset of the rules, so do not claim full compliance.

---

## Phase 9: AUTOSAR Classic + Safety Concepts (Weeks 18-20) ⭐⭐⭐

Concept level is what a junior needs. You are being hired to understand where your code sits, not to configure a full stack on day one.

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 18 | AUTOSAR Classic architecture ⭐⭐⭐ | Map your own code onto the layers: MCAL, ECU abstraction (CanIf), services (ComM, Dcm, Dem, NvM, EcuM), RTE, SWCs | Comm stack: CAN driver -> CanIf -> PduR -> Com / CanTp -> Dcm; ARXML + config tools (awareness); AUTOSAR OS vs FreeRTOS (OSEK heritage: tasks, alarms, ISR categories) | One-page diagram: "my UDS stack in AUTOSAR terms" |
| 19 | Mini AUTOSAR-style CAN stack ⭐⭐ | CanIf-lite + PduR-lite + Com-lite (DBC signals) wired to your Dcm-lite (your UDS server) | DTC lifecycle: pending/confirmed, debouncing, freeze frame, NvM | **Portfolio: mini-autosar-comstack-c** (small) |
| 20 | Safety + process vocabulary ⭐⭐⭐ | Write a short requirements list for your watchdog module, trace each requirement to a unit test, and write a one-page FMEA | ISO 26262 overview (parts, ASIL A-D, HARA, safety goal, FTTI, diagnostic coverage); ASPICE and V-model (SWE.1-SWE.6 at a glance, traceability); cybersecurity awareness (ISO 21434, UN R155); one evening: Lauterbach TRACE32 awareness (what it is, what a debug/trace workflow looks like) | Requirements-to-tests trace table |

You do not build an "ASIL-B-ready module". Interviewers test vocabulary and understanding, not a safety case.

**MUST milestone (end of Week 20): everything marked ⭐⭐⭐ is done.**

---

## Phase 10: NXP S32K144 (Weeks 21-23) ⭐⭐ (hedge, only if the gate says yes)

**Decision gate at Week 16.** Buy the S32K144EVB-Q100 only if your posting tally or interview feedback points at NXP S32K or similar automotive MCUs (S32K, AURIX and RH850 counts from the Week 1 survey). If the numbers don't support it, take the no-board fallback below. If you do buy, order around Week 16-18 (lead times fluctuate); price is about 105 USD / about 104 EUR ex VAT at distributors, so roughly 128 EUR with Polish VAT (my estimate; verify when ordering).

What the board gives you: an automotive MCU family, native on-chip CAN (no MCP2515, no level shifter), an on-board system basis chip with high-speed CAN, a LIN transceiver, and an OpenSDA debugger. Goal: exposure plus proof that your Week 8 layering is portable. Do not switch platforms. S32K1 is the older generation; newer designs use S32K3.

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 21 | Bring-up | Install S32 Design Studio for Arm; register-level LED + LPUART without any SDK; then the same via NXP's current driver package for S32K1 and compare | Reference-manual chapters on clocks, PORT, GPIO; **flash configuration field near the start of flash: read the RM before touching it, wrong values can secure the chip** | LED + UART hello, startup/linker differences vs STM32 |
| 22 | FlexCAN + LIN | FlexCAN with message buffers, bit timing from the clock, filters; port your CAN sensor node + ISO-TP + UDS server (only the driver layer should change); real LIN bus test with the on-board transceiver | FlexCAN vs bxCAN vs MCP2515; the SBC has its own modes and watchdog: read its datasheet and the EVB user guide before the CAN transceiver will work | UDS server running on S32K144 |
| 23 | Flash + port | Port the bootloader's flash driver (different erase unit and programming granularity); verify the UDS flash sequence | Cross-MCU comparison table (clock tree, peripherals, flash, debug) | **Portfolio: s32k144 port** of UDS server + bootloader; comparison README |

CAN FD: I could not confirm CAN FD on this board. Check which FlexCAN instance supports FD and whether the EVB's transceiver does before relying on it. Otherwise stay at the concept level from Week 13.

**No-board fallback:** (a) 1 week reading the S32K/AURIX/RH850 reference-manual structure (what differs from STM32), (b) 1 week on SocketCAN on Linux (CAN FD, `candump`, `cansend`), (c) 1 week extra buffer or pipeline time.

---

## Phase 11: C++ for Embedded, Compressed (Weeks 24-27) ⭐⭐ (hedge)

Use `arm-none-eabi-g++` with `-fno-exceptions -fno-rtti`. No STL containers, no heap. Goal: honest basics, not mastery. If the Week 1 survey shows C++ in the majority of postings, move these weeks up to right after Week 17.

| Week | Topic | Morning Practice | Evening Theory | Project |
|---|---|---|---|---|
| 24 | Classes, RAII, references, `enum class`, `constexpr`, `static_assert` | Port your LED/GPIO C driver to a C++ class; `enum class` for register fields | Constructors/destructors, RAII for hardware, references vs pointers | GPIO + LED classes |
| 25 | Templates + zero-cost abstraction | `RingBuffer<T, N>`; template functions; `constexpr` CRC table | Compare code size and speed against the C version (`arm-none-eabi-size`, objdump) | RingBuffer<T,N> + benchmark |
| 26 | RAII for hardware + error handling | SPI chip-select guard, mutex lock guard, scope guard; small `Optional<T>` or the Embedded Template Library (ETL) | Error handling without exceptions | UART + SPI classes |
| 27 | Real module port + interop | CAN frame class; port ISO-TP or the CRC/flash module; FreeRTOS C++ wrappers (Task, Queue, Mutex); `extern "C"` between C and C++ | MISRA C++:2023 awareness (it absorbed the AUTOSAR C++14 guidelines, so no separate compliance study) | **Portfolio: stm32-cpp-hal** (small and honest) |

Done bar: a CAN + UART + SPI C++ layer next to your C code, zero heap use, code size within a few percent of C.

---

## Phase 12: Interview Consolidation (Weeks 28-30) ⭐⭐⭐

Drills have been running since Week 12. These weeks consolidate and test.

| Week | Focus | Deliverable |
|---|---|---|
| 28 | Embedded C drills: `volatile`, `const`, pointers, bit manipulation, endianness, struct padding, memory layout (.text/.data/.bss), stack vs heap, ISR rules, race conditions, atomicity. Write by hand: ring buffer, bit reverse, memcpy, linked list, CRC | 30 questions answered out loud, 10 coded on paper |
| 29 | Protocol + RTOS drills: CAN arbitration/bit stuffing/error states, UDS/ISO-TP walkthroughs, FreeRTOS (priority inversion, deadlock, ISR to task), "design a door-module ECU". Mock interviews (peer or recorded) using STAR for your flagship projects. Practise English technical explanations | 2 mock interviews, feedback written down |
| 30 | Offers and positioning: GitHub Pages portfolio site (flagship repos, demo videos), LinkedIn keywords, salary and contract research (Poland: employment vs B2B; Germany: Blue Card, relocation, language expectations) | Portfolio site live, offer checklist |

## Buffer + Specialization (Weeks 31-33) ⭐

Use for slippage first. If on schedule, pick one item from the post-plan list below, or one thing a target employer's job ads asked for repeatedly.

---

## CV projects: what goes on the CV and in what order

Put 4-5 projects on the CV, strongest first. Link repos with a README, architecture diagram, and a demo capture. Do not list exercises (LED blink, calculator, PID simulation) as projects.

| Priority | Repo | Available | Proves |
|---|---|---|---|
| 1 (flagship) | **uds-bootloader-c** | Week 15 | Reflashing over CAN with ISO-TP + UDS, what production ECUs do |
| 2 | **iso-tp-c + uds-server-c** | Week 11 | ISO-TP, UDS services, pytest-tested with Python |
| 3 | **rtos-can-sensor-hub-c** | Week 7 | RTOS task design, deferred interrupt processing, CAN |
| 4 | **stm32-can-sensor-node-c** | Now (polish in Week 8) | I2C + CAN + interrupts, signal design with counter and CRC |
| 5 | **stm32-bootloader-c** | Now (polish in Week 8) | Bare-metal boot, VTOR, flash |
| Supporting | obd2-simulator-c | Week 12 | OBD-II basics |
| Supporting | mini-autosar-comstack-c | Week 19 | AUTOSAR layering understanding |
| Conditional | s32k144-port | Week 23 | Same stack on an automotive MCU (only if the gate said yes) |
| Hedge | stm32-cpp-hal | Week 27 | Honest C++ basics. Move up if postings demand C++ |

Before Week 15, the first applications link to the CAN node, the bootloader, the RTOS hub and the UDS server. From Week 17, every repo shows a green CI badge. Honest CV wording for tooling: "applied MISRA C:2012 guidelines using static analysis, Unity unit tests, GitHub Actions CI".

---

## What got cut and why

| Old content | Decision | Why |
|---|---|---|
| RISC-V, TinyML | Cut | Rarely required in junior automotive postings. Learn on the job if needed. |
| 8-week safety phase, ASIL-B module | Replaced by Week 20 concepts + MISRA/tests in Weeks 16-17 | Juniors are tested on vocabulary and discipline, not safety cases. |
| 8-week C++ phase + 2 AUTOSAR C++14 weeks + C++20/23 | Compressed to 4 weeks | "C/C++" shows up in postings, but the bulk of junior Classic AUTOSAR ECU work is C. AUTOSAR C++14 was merged into MISRA C++:2023. |
| QNX, C++ in ASIL-D | Cut | Senior/specialist topics. |
| FreeRTOS tickless week | Cut | Low-power work is already covered by your Week 19 on STM32. |
| 3-week design patterns phase | Folded into Week 8 | Layering matters; the rest is theory. |
| Virtual development kits | Skip for now | Learn when an employer uses them. |
| Post-plan items DBC code generator, Dem/NvM-lite, cross-MCU C++ HAL | Cut | Out of scope for a junior hiring bar. |

## Post-plan / buffer picks (in this order)

1. **Test automation rig.** `pytest` + `python-can` drives the real ECU: UDS regression, bootloader flashing, wrong sequence numbers, run from CI. Nearly free, since the pytest suites from Weeks 11 and 15 already exist. Matches Poland's many validation/test roles.
2. **Mini car network demo** (only if you bought the S32K board): STM32 + S32K144 + CANable, one DBC, UDS on every node, a Python tool running a diagnostic session across the network, demo video.
3. **Secure bootloader v2:** signed firmware (Ed25519 or ECDSA with an existing crypto library, do not write your own), anti-rollback counter. Cybersecurity (ISO 21434, UN R155) matters to German OEMs and Tier-1s, but treat it as a bonus, not a prerequisite.

After you are hired (learn what the team needs): Adaptive AUTOSAR / SOME-IP / DoIP and automotive Ethernet, AURIX or RH850, deeper ISO 26262 work, Rust for embedded, RISC-V, TinyML, virtual development kits.

---

## One-sentence summary

Finish what you started, learn the protocols jobs are built on (ISO-TP, UDS, OBD-II, DBC), prove it with a UDS bootloader, add tooling discipline (MISRA, tests, CI) and AUTOSAR/safety vocabulary, start applying at Week 12 with interview drills running alongside, and treat C++ and the second MCU as hedges decided by what the postings and interviews actually ask for.
