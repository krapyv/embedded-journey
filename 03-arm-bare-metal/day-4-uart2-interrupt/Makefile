# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
INCLUDES = -I. -Ilibraries

# Compiler flags (same for all STM32F411 projects)
CFLAGS = -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles
LDFLAGS = -T stm32f411.ld

# Project files (CHANGE THIS FOR EACH PROJECT)
SRC = uart.c libraries/ring_buffer.c libraries/led.c startup_stm32f411ceux.s
ELF = program.elf
BIN = program.bin

# Default target
all: $(BIN)

# Link: .c and .s -> .elf
$(ELF): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(SRC) -o $(ELF)

# Convert: .elf -> .bin
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $(ELF) $(BIN)

# Flash using ST-LINK
flash: $(BIN)
	STM32_Programmer_CLI -c port=SWD -w $(BIN) 0x08000000 -v -rst

# Flash using st-flash (alternative)
flash-st: $(BIN)
	st-flash --reset write $(BIN) 0x08000000

# Clean build artifacts
clean:
	rm -f $(ELF) $(BIN)

# Mark these as not real files
.PHONY: all flash flash-st clean
