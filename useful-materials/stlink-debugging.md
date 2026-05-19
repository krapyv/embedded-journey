Here is a comprehensive, step-by-step documentation template tailored for your `.md` file. It breaks down the exact workflow you just mastered so you can copy, paste, and refer back to it instantly in the future.

---

# Bare-Metal STM32 Debugging Guide

This guide details the process for flashing and debugging unoptimized bare-metal C applications on an STM32F411 BlackPill using an ST-LINK programmer over SWD on Linux.

## 1. Prerequisites & Prerequisites Setup

### Compilation Requirements

To ensure GDB maps machine code back to readable C lines and preserves delay loops, compile using the `-g` (debug symbols) and `-O0` (zero optimization) flags:

```bash
arm-none-eabi-gcc -g -O0 -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -T stm32f411.ld startup_stm32f411ceux.s led_blink.c -o blink.elf

```

---

## 2. Step-by-Step Debugging Execution

Always utilize two terminal windows or tabs: **Terminal 1** handles the background hardware link, and **Terminal 2** runs your interactive GDB workspace.

### Step 2.1: Start the Background GDB Server (Terminal 1)

Connect your ST-LINK clone and BlackPill to your PC's USB ports. In your first terminal window, start the local gdbserver backend:

```bash
st-gdb

```

* **Expected Output:** The terminal should print configurations showing `Listen Port Number : 61234` and sit at `Waiting for debugger connection...`. The ST-LINK COM LED will flash red and green.

### Step 2.2: Launch and Link GDB (Terminal 2)

Open a separate terminal window, navigate to your active compilation directory, and execute the following sequence:

1. **Launch the GDB Client pointing to your ELF target:**
```bash
arm-none-eabi-gdb blink.elf

```


2. **Connect to the waiting backend server port:**
```text
(gdb) target remote :61234

```


3. **Flash the updated binary down to the chip's internal memory:**
```text
(gdb) load

```


4. **Set a breakpoint at the entry point of your main logic:**
```text
(gdb) break main

```


5. **Boot the processor directly to your breakpoint:**
```text
(gdb) continue

```


*The execution will halt exactly at the first line inside your `main()` loop.*

---

## 3. Essential GDB Cheat Sheet

| Command | Shortcut | Description |
| --- | --- | --- |
| `list` | `l` | Displays surrounding lines of readable C code. |
| `next` | `n` | Steps over the current line of code (treats function calls/loops as single steps). |
| `step` | `s` | Steps directly *into* a function call (e.g., jumps inside a delay function). |
| `continue` | `c` | Resumes unrestricted hardware-speed execution on the core. |
| `finish` | `fin` | Runs full-speed until the current function ends, returning back out to the caller. |
| `advance [line]` | — | Force-runs full speed directly until reaching a specific line number. |
| `Ctrl + C` | — | Interrupts full-speed execution, freezing the CPU wherever it is currently spinning. |
| `quit` | `q` | Safely disconnects the debugger and exits GDB. |

### Inspecting Memory Registers on STM32F411

Because hardware registers are memory-mapped, you can check peripheral states live using direct dereferencing:

* **Read Clock Gating (`RCC_AHB1ENR`):**
```text
(gdb) print /x *(unsigned int *)0x40023830

```


* **Read Port A Pin Modes (`GPIOA_MODER`):**
```text
(gdb) print /x *(unsigned int *)0x40020000

```


* **Read Port A Output States (`GPIOA_ODR`):**
```text
(gdb) print /x *(unsigned int *)0x40020014

```



---

## 4. Troubleshooting Breakpoints

If you set temporary debugging breakpoints (e.g., `break 64`) to bypass delay cycles and need to remove them later to let the program loop infinitely:

* **View all active breakpoints and their assigned IDs:**
```text
(gdb) info break

```


* **Delete a specific breakpoint (e.g., ID 2):**
```text
(gdb) d 2

```


* **Wipe all breakpoints cleanly to let the program loop without interruption:**
```text
(gdb) delete

```



---

## 5. Clean Exit Routine

When closing down your desk workstation setup, follow this order to prevent locked serial sockets or background processes:

1. Inside **Terminal 2** (GDB client): Press `Ctrl + C` to pause the core, type `q`, and hit Enter.
2. Inside **Terminal 1** (GDB server): Press `Ctrl + C` to terminate the active `st-gdb` session cleanly.
