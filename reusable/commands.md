COMPILATION:

arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -T stm32f411.ld startup_stm32f411ceux.s led_blink.c -o blink.elf

CONVERT TO BINARY:

arm-none-eabi-objcopy -O binary blink.elf blink.bin
