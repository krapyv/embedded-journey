drake21@drake21-Inspiron-7577:~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin$ ./STM32_Programmer_CLI -c port=SWD -r 0x08000000 0x1000 dump.bin
      -------------------------------------------------------------------
                        STM32CubeProgrammer v2.22.0                  
      -------------------------------------------------------------------

ST-LINK SN  : 49FF70065050825529331767
ST-LINK FW  : V2J29S7
Board       : --
Voltage     : 3.23V
SWD freq    : 4000 KHz
Connect mode: Normal
Reset mode  : Software reset
Device ID   : 0x431
Revision ID : Rev A
Device name : STM32F411xC/E
NVM size  : 512 KBytes
Device type : MCU
Device CPU  : Cortex-M4
BL Version  : 0xD0


UPLOADING ...
  File          : dump.bin
  Size          : 4096 Bytes
  Address:      : 0x8000000
Read progress:
[==================================================] 100% 

Data read successfully
Time elapsed during read operation is: 00:00:00.033


drake21@drake21-Inspiron-7577:~/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin$ hexdump -C dump.bin | head -20
00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
*
00001000


-------------------------------------------------------------------
drake21@drake21-Inspiron-7577:/mnt/hdd/embedded-journey/03-arm-bare-metal/day-2-led-blink-basic-working$ arm-none-eabi-gdb blink.elf
GNU gdb (GNU Tools for STM32 14.3.rel1.20251027-0700) 15.2.90.20241229-git
Copyright (C) 2024 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-linux-gnu --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from blink.elf...
(gdb) target remote :61234
Remote debugging using :61234
Reset_Handler () at startup_stm32f411ceux.s:58
58	  ldr   r0, =_estack
(gdb) load
Loading section .text, size 0x290 lma 0x8000000
Start address 0x080001ac, load size 656
Transfer rate: 1 KB/sec, 656 bytes/write.
(gdb) break main
Breakpoint 1 at 0x8000234: file led_blink.c, line 23.
Note: automatically using hardware breakpoints for read-only addresses.
(gdb) continue
Continuing.

Breakpoint 1, main () at led_blink.c:23
23	    RCC_AHB1ENR |= 1; // the formula is RCC_AHB1ENR |= 1 << n, but n = 0 (because we are changing the first bit
(gdb) list
18	    // first of all, we need to turn the clock enable
19	    // we already have the macro that gives use the value at the clock address
20	    // according to the reference manual the bit 0 is the bit that enablea/disables clock for the port A
21	    // so we need to flip the bit 0 to value 1
22	
23	    RCC_AHB1ENR |= 1; // the formula is RCC_AHB1ENR |= 1 << n, but n = 0 (because we are changing the first bit
24	
25	    // next we want to set a direction (GPIOA_MODER)
26	    // every pin has 2 bits that represent its GPIO_MODER register
27	    // the first bit can represent the mode 00 - input (reset state) and 01 - the General Input output mode (the one we want)
(gdb) n   
39	    GPIOA_MODER &= ~(0x3 << 10); // the bits 10-11 are 0 in binary
(gdb) print /x *(unsigned int *)0x40023830
$1 = 0x1
(gdb) n
44	    GPIOA_MODER |= 0x1 << 10;
(gdb) n
52	        GPIOA_ODR |= 1 << 5; // GPIOA_ODR |= 1 << n where n = number of pin of the
(gdb) print /x *(unsigned int *)0x40020000
$2 = 0xa8000400
(gdb) n
57	        delay(300000);
(gdb) print /x *(unsigned int *)0x40020000
$3 = 0xa8000400
(gdb) print /x *(unsigned int *)0x40020000      
$4 = 0xa8000400
(gdb) print /x *(unsigned int *)0x40020014
$5 = 0x20
(gdb) continue
Continuing.
^C
Program received signal SIGINT, Interrupt.
0x08000220 in delay (count=300000) at led_blink.c:11
11	    for (volatile uint32_t i = 0; i < count; i++)
(gdb) n
n
continue
^C
Program received signal SIGINT, Interrupt.
0x08000216 in delay (count=300000) at led_blink.c:11
11	    for (volatile uint32_t i = 0; i < count; i++)
(gdb) continue
Continuing.
advance 20
fin
^C
Program received signal SIGINT, Interrupt.
0x08000216 in delay (count=300000) at led_blink.c:11
11	    for (volatile uint32_t i = 0; i < count; i++)
(gdb) advance 20
main () at led_blink.c:52
52	        GPIOA_ODR |= 1 << 5; // GPIOA_ODR |= 1 << n where n = number of pin of the
(gdb) advance 59
main () at led_blink.c:64
64	        GPIOA_ODR &= ~(1 << 5);
(gdb) break 64
Breakpoint 2 at 0x800026a: file led_blink.c, line 64.
(gdb) continue
Continuing.

Breakpoint 2, main () at led_blink.c:64
64	        GPIOA_ODR &= ~(1 << 5);
(gdb) n
66	        delay(300000);
(gdb) continue
Continuing.

Breakpoint 2, main () at led_blink.c:64
64	        GPIOA_ODR &= ~(1 << 5);
(gdb) continue
Continuing.

Breakpoint 2, main () at led_blink.c:64
64	        GPIOA_ODR &= ~(1 << 5);
(gdb) info break
Num     Type           Disp Enb Address    What
1       breakpoint     keep y   0x08000234 in main at led_blink.c:23
	breakpoint already hit 1 time
2       breakpoint     keep y   0x0800026a in main at led_blink.c:64
	breakpoint already hit 3 times
(gdb) d 2
(gdb) info break
Num     Type           Disp Enb Address    What
1       breakpoint     keep y   0x08000234 in main at led_blink.c:23
	breakpoint already hit 1 time
(gdb) continue  
Continuing.
^C
Program received signal SIGINT, Interrupt.
0x08000218 in delay (count=300000) at led_blink.c:11
11	    for (volatile uint32_t i = 0; i < count; i++)
(gdb) 


