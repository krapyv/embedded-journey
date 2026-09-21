# Debug console (stack + UART + RPN EVAL)

The project implements the input-handling of a UART debug console (HELP/ECHO/EVAL (with Reversed Polish Notation) commands) sitting on top of an existing interrupt-driven ring-buffer RX. Two buffers involved: a 32-byte `line_assembly_buf` (31 usable chars + 1 reserved terminator slot) and a `char* tokens[16]` array of pointers into that buffer.
The project is built on top of the UART and Stack reusable drivers.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│     ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│     └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
│
├── periph/     # Portable peripheral drivers
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    └── uart.h      # USART2 bit definitions, control macros, and function declarations
│
├── utils/     # Reusable data-structure components
│    ├── stack.c       # Stack initialization, push, pop and clear functions
│    ├── stack.h       # Stack handle struct and function headers
│    ├── ring_buffer.c       # Enqueue, dequeue, flush, and status function implementations
│    └── ring_buffer.h       # Configuration handle layouts and buffer tracking structures 
│
├── main.c      # Line buffer assembly, tokenizer, dispatcher, commands implementations, error handling
├── app_config.h    # The header file for UART peripheral mode, classificator enum
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
└── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM

```

## Architecture Decisions

1. Newline-terminated fixed-size line buffer with reject-on-overflow, not the chunked bootloader protocol - a human typing can't compute a checksum or track a length field the way a host tool can.

2. `Stack_t.*data` stays `uint32_t`.
Traced `3 - 4` in two's complement to confirm no information is lost on store, just reinterpreted (`0xFFFFFFFF` reads as `-1` signed or `4294967295` unsigned) - an (int) cast at print time flips the interpretation. Verified `+ - *` produce identical bit patterns regardless of signed/unsigned typing (pure modular arithmetic), but `/` genuinely diverges - Cortex-M4 emits a different instruction (UDIV vs SDIV). For `/` operation, both operands are casted to `int32_t` type during evaluation.

3. Pointers into the line buffer (`char *tokens[16]`, boundaries null-terminated in place) over a copied 2D array - checked the actual hazard (pointer invalidation if the buffer mutates while pointers are still live) against the actual control flow (strictly sequential: read -> parse -> execute, no re-entry) and confirmed it can't occur here.

4. Dispatch uses plain `if/else if` chain over a function-pointer table - a table only earns its complexity at a command count or turnover rate this project doesn't have.

5. Line assembly has to reject an oversized write before it happens. Its buffer sizing: 32-byte buffer = 31 usable chars + 1 reserved terminator slot. Verified the boundary by hand: `write_idx == 30` passes, lands at `buffer[30]` (the 31st char), only then increments to 31, and the next byte's gate correctly fails before touching the terminator slot.

6. Tokenizer boundary condition: "this character starts a token" derived as `prev_was_space && current != space` - a two-part condition, not just "not a space" (needed to correctly reject the second space in a double-space run). Index 0 handled by seeding `prev_was_space = true` before the loop starts, standing in for a nonexistent index -1. '\0' is written at every detected token boundary to separate all the tokens.

7. `classify_token` function uses length-based branching: 
- length >= 2 -> must be a multi-digit or signed number; can never be an operator, since every defined operator is exactly one character - a hard guarantee from the grammar itself, not a guess.
- length == 1 -> ambiguous by length alone ("-", "+", "5" are all length 1 (the second character is a null terminator which is not counted by `strlen()`); resolved by checking the single real character's identity against the operator set, then against digits.
- length == 0 -> just '\0', an empty token, is not counted by the `strlen()`. Falls through to invalid by construction.

8. `strtol` instead of the hand-rolled digit scan.
Switched from a manual per-character remainder scan to `strtol(token, &endptr, 10)` plus checking `*endptr == '\0'` - if the whole token parsed as a number with nothing left over, it's valid; if `endptr` stops short (e.g. "2a2"), it's garbage.

9. ECHO reconstructs the user's words (`token[1..token_count-1]`) and prints them back with spacing restored without reconstructing the double-spacing, since the tokenizer has already destroyed the original spacing by overwriting every space with '\0' as a boundary marker.

10. Since minicom sends '\r' (0x0D, carriage return) on Enter - a terminal-configuration behavior (line-ending setting), - the firmware's line-completion check is decided to be `popped_byte == '\r'`.

11. `Stack_push`'s guard was `stack->top >= stack->size - 1`, with `top` declared `int` and `size` declared `uint32_t`. C's usual arithmetic conversions promote the signed operand to unsigned before a mixed comparison runs - so on a freshly-cleared, empty stack (`top == -1`), the comparison doesn't see `-1`, it sees `-1` reinterpreted bit-for-bit as `0xFFFFFFFF` (`4294967295`), the largest possible `uint32_t` value. Fixed with an explicit cast to keep both operands signed for the comparison: `stack->top >= (int)stack->size - 1`.

12. While constructing a deliberate test for the 16-token error path, discovered the two overflow limits (31-char line buffer, 16-token array) can never both be exercised independently with single-character tokens - one always masks the other.
So decided to leave both buffer sizes as-is rather than artificially loosening one of them just to manufacture a passing test that would need to be reverted afterward - treated proving the path structurally unreachable, with exact arithmetic, as an equally valid way to satisfy the spec's requirement that the condition be "handled".

## Breadboard circuit

The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

## Known Limitations

1. Two overflow limits (31-char line buffer and 16-token array overflow guards) cannot be independently exercised by a test using single-character tokens - one guard always fires before the other becomes reachable. 
Both guards are structurally correct (proven in Architecture Decisions 12); this is a test-coverage gap, not a functional one. A multi-character-token test case would be needed to exercise the 16-token guard in isolation.

2. No printing to the terminal as a user types, no deletion functionality - bad UI/UX-experience.

3. Integer number support only, hardcoded 4 operations (`+ - * /`).
