Useful STM32CubeProgrammer Commands (From Any Folder)
Action	Command
Flash and run	STM32_Programmer_CLI -c port=SWD -w blink.bin 0x08000000 -v -rst
Erase all flash	STM32_Programmer_CLI -c port=SWD -e all
Read flash to file	STM32_Programmer_CLI -c port=SWD -r 0x08000000 0x1000 dump.bin
Connect only (test)	STM32_Programmer_CLI -c port=SWD
Reset MCU	STM32_Programmer_CLI -c port=SWD -rst
