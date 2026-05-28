# Ring Buffer Reusable Library

This project provides a highly reusable, thread-safe-conscious, and hardware-agnostic implementation of a Ring (Circular) Buffer in standard C. It is designed specifically for bare-metal embedded applications, such as handling asynchronous UART transmission/reception queues, decoupling high-frequency interrupt service routines (ISRs) from low-priority background processing loops.
This implementation contains zero vendor HAL overhead, uses static allocation safely through custom control structures, and operates cleanly across any ARM Cortex-M or alternative microcontroller architecture.

## 📂 Project Structure

```text
├── ring_buffer.c       # Enqueue, dequeue, flush, and status function implementations
├── ring_buffer.h       # Configuration handle layouts and buffer tracking structures   
└── main.c              # Test application demonstration FIFO queue verification

```

## 🧠 Core Low-Level Concepts
### First-In, First-Out (FIFO) Architecture

A Ring Buffer wraps a fixed-size contiguous memory block around itself using two tracking indices: a `head` (where data is written) and a `tail` (where data is read).

* **Head Pointer:** Tracks the next available slot for incoming data. Incremented on a push (`enqueue`) operation.
* **Tail Pointer:** Tracks the oldest unread byte in the buffer. Incremented on a pop (`dequeue`) operation.
* **Wrap-Around Mechanics:** Instead of shifting elements in memory when data is read, the tracking indices wrap back to index `0` when they hit the buffer boundary using a modulo control mask.

### Atomic Protection and Boundary Conditions

To prevent data corruption when an Interrupt Service Routine (ISR) preempts the main thread, the library flags full and empty conditions cleanly without index collision:

1. **Buffer Empty State:** 
The buffer is inherently empty when the tracking indices point to the exact same memory position: 

$$\text{Head} == \text{Tail}$$

2. **Buffer Full State:**
To distinguish between completely empty and completely full states without wasting extra status flags, this implementation leaves one slot empty as a boundary delimiter. The buffer is filled when the next write index calculation matches the read index:

$$((\text{Head} + 1) \pmod{\text{Size}}) == \text{Tail}$$

## 🛠️ Memory Management & Modular Abstraction

The driver avoids hardcoded sizes and global arrays by storing execution metrics entirely inside a dedicated configuration structure:

```c
typedef struct
{
    uint8_t *buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t max_size;
} RingBuffer_t;

```

### Optimal Index Wrap-Around Masking

Instead of executing an expensive hardware modulo operation (`%`) during every read and write cycle, the driver optimizes index wrapping by forcing power-of-two buffer constraints ($2^n$). This transforms the calculation into a highly optimized, single-cycle bitwise AND operation:

```c
rb->head = (rb->head + 1) & (rb->max_size - 1);
```

## 🚀 Compilation and Integration

This repository contains both the core modular library files and a desktop-capable validation testbench. The implementation can be compiled natively for logic testing or cross-compiled for embedded target integration.

### 1. Running the Desktop Testbench
To verify the FIFO boundary limits, pointer wrapping, and safety guards without physical silicon hardware, compile and execute the mock testbench natively on your computer using a standard host compiler (`gcc`, `clang` etc.):

```bash
gcc main.c ring_buffer.c -Wall -O2 -o ring_buffer_testbench

./ring_buffer_testbench
```

#### Output:
```text
=== Starting Ring Buffer Desktop Testbench ===

[PASS] Initialization safely rejected invalid size 7!
[PASS] Initialization succeeded for size 4.

--- Testing Full Capacity Boundary ---
Pushed 'A' | Head offset: 1, Tail offset: 0
Pushed 'B' | Head offset: 2, Tail offset: 0
Pushed 'C' | Head offset: 3, Tail offset: 0
Rejected 'D' | Buffer is FULL! (Head: 3, Tail: 0)

--- Testing Pop / Emptying ---
Popped: 'A' | Head offset: 3, Tail offset: 1
Popped: 'B' | Head offset: 3, Tail offset: 2
Popped: 'C' | Head offset: 3, Tail offset: 3
Buffer is now completely empty.

--- Testing Wrapped Chase State ---
Pointers moved up. Current Head: 1, Tail: 1
Pushing characters to trigger bitmask rollover:
Pushed '1' | Head rolled over to offset: 2
Pushed '2' | Head rolled over to offset: 3
Pushed '3' | Head rolled over to offset: 0

=== Testbench Execution Complete ===
```

### 2. Embedded Target Deployment (Production Linking)
To deploy this module inside an active firmware implementation, omit `main.c` from your build pipeline and compile `ring_buffer.c` directly into an isolated object file (`.o`) via a cross-compiler toolchain:

#### Compile as a Hardware-Agnostic Object File

```bash
arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb -Wall -O2 ring_buffer.c -o ring_buffer.o
```

* `-c`: instructs the compiler to output the raw machine code object file (`ring_buffer.o`) and bypass the linking stage.
* `-mcpu=cortex-m4 -mthumb`: Formats execution instructions explicitly for the target ARM microcontroller hardware.

### Integrate and Link Into Target Application

To use this library in an application, include the header file in your main application file and link the compiled object files together.

```bash
arm-none-eabi-gcc app_main.o ring_buffer.o uart.o -mcpu=cortex-m4 -mthumb -T linker_script.ld -o final_application.elf
```

## 🗺️  Next Steps & Learning Roadmap

To make this driver fully production-ready for automotive deployment, the next development steps for this library are:
1. **MISRA C Compliance**: Enforce exact type safety casting on index transformations and rule out any implicit wrapping assignments.
2. **Critical Section Abstraction**: Implement architectural-level disabling hooks (`__disable_irq()`, `__enable_irq()`) inside data modifications to guarantee absolute atomicity during multi-byte enqueue operations.
3. **Watermark Threshold Alarms:** Add runtime diagnostic callbacks that alert the application layer if the buffer fills beyond 80% capacity, preventing critical overflow data loss.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
