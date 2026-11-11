# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy

# --- Centralized Driver Path ---
DRIVERS = ../../my_reusable_drivers
FREERTOS = ../../third_party/FreeRTOS-Kernel
CONFIG = ./config

INCLUDES = -I. -I${DRIVERS}/core -I${DRIVERS}/devices/led -I${DRIVERS}/periph/uart -I${CONFIG} -I${FREERTOS}/include -I${FREERTOS}/portable/GCC/ARM_CM4F 

# Compiler flags (same for all STM32F411 projects)
CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -nostartfiles -g3 -O0 -Wundef -Wall -Wextra --specs=nano.specs --specs=nosys.specs -lc -DUSE_APP_CONFIG
LDFLAGS = -T stm32f411.ld

# Project files (CHANGE THIS FOR EACH PROJECT)
SRC = startup_stm32f411ceux.s ${DRIVERS}/periph/uart/uart.c ${DRIVERS}/devices/led/led.c ${DRIVERS}/periph/uart/ring_buffer/ring_buffer.c ${FREERTOS}/tasks.c ${FREERTOS}/list.c ${FREERTOS}/queue.c ${FREERTOS}/portable/GCC/ARM_CM4F/port.c ${FREERTOS}/portable/MemMang/heap_4.c main.c
ELF = program.elf
BIN = program.bin

# Default target
all: $(BIN)

# Link: .c and .s -> .elf
$(ELF): $(SRC) $(CONFIG)/FreeRTOSConfig.h
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