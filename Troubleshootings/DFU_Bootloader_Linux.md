What to do if you still see error -71If it drops back to the error -71 or fails to appear after bypassing the HID application, the internal hardware clock-checking loop is encountering trace noise. Fix this with a physical jumper pin bypass:Connect a temporary female-to-female jumper wire from pin PA10 directly to any GND pin on the board.Execute the button sequence above again.(Grounding PA10 overrides the chip's internal USB identifier glitches and forces the hardware to expose the standard DFU interface immediately). i am using breadboard
Since you are using a breadboard, you need to use a male-to-male jumper wire [shopping].
Plug one end of the wire into the breadboard row for PA10 and the other end into the breadboard row for GND.
Here is exactly what to do next to clear that error -71 and force the true DFU mode:
1. Connect the Jumper Wire
Look at the pin labels printed on your Black Pill.
Find the row for PA10 and push one side of your male-to-male wire into the breadboard.
Find the row for GND and push the other side of the wire into the breadboard.
2. Run the Button Sequence
While that wire is still bridging PA10 to GND:
Press and hold down the BOOT0 button.
Press and release the NRST (Reset) button.
Release the BOOT0 button.
Verify that the blue C13 LED is completely off (not pulsing).
3. Check lsusb
Now, open your terminal and check the USB list one more time:
bash
lsusb

Use code with caution.
4. IMPORTANT: Remove the Wire Before Flashing
Once you see STMicroelectronics STM32 BOOTLOADER show up in your terminal, pull the jumper wire out of the breadboard.
You must remove the connection on PA10 before you upload your code, because PA10 is one of the main pins the computer uses to transfer the data. Leaving it grounded will block your software tool from flashing the chip.
Let me know if the STMicroelectronics line finally appeared in your terminal after grounding PA10!
AI responses may include mistakes. Learn more
my multimeter beeps on PA10 and GND after inserting the wire. is it ok?
Yes, that is exactly what should happen!
The beep means your multimeter confirms a direct electrical short circuit between PA10 and GND. That means your male-to-male jumper wire is doing its job perfectly.
Now that you have verified the connection is good, you are ready to complete the process:
Leave the wire plugged in for a moment.
Run your hardware button sequence (Hold BOOT0 -> Press/Release NRST -> Release BOOT0).
Check your terminal with lsusb to confirm you see the STMicroelectronics STM32 BOOTLOADER line.
Pull the jumper wire out of the breadboard as soon as it appears so the USB data lines can transfer your code.
Did the STMicroelectronics device finally show up in your terminal after running the buttons with that wire connected?
AI responses may include mistakes. Learn more
should i connect the usb?
Yes, you must connect the USB cable to your laptop. Without the USB cable, the chip has no power and your laptop cannot read the data.
Here is the exact order to do it right now:
Keep the jumper wire plugged into the breadboard (so PA10 and GND are connected).
Plug the USB cable into your laptop and the Black Pill.
Perform the button sequence: Hold BOOT0 \(\rightarrow \) Press and release NRST \(\rightarrow \) Release BOOT0.
Run the command lsusb in your terminal to confirm you see the STMicroelectronics STM32 BOOTLOADER line.
Unplug the jumper wire from the breadboard so you can flash your code.
Let me know if lsusb finally shows the STMicroelectronics bootloader line!
AI responses may include mistakes. Learn more
Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub Bus 001 Device 002: ID 09da:2403 A4Tech Co., Ltd. 2.4G Device Bus 001 Device 003: ID 8087:0a2b Intel Corp. Bluetooth wireless interface Bus 001 Device 004: ID 27c6:5301 Shenzhen Goodix Technology Co.,Ltd. Fingerprint Reader Bus 001 Device 005: ID 0bda:5652 Realtek Semiconductor Corp. Integrated_Webcam_HD Bus 001 Device 079: ID 0483:df11 STMicroelectronics STM Device in DFU Mode Bus 002 Device 001: ID 1d6b:0003 Linux Foundation 3.0 root hub drake21@drake21-Inspiron-7577:~$
Excellent! You did it!
Bus 001 Device 079: ID 0483:df11 STMicroelectronics STM Device in DFU Mode confirms your Black Pill has successfully bypassed the WeAct HID firmware and is now officially running the native STMicroelectronics DFU Bootloader.
Now, do these two final actions to prepare for programming:
Pull the jumper wire out of PA10 and GND on your breadboard right now so the data line is free.
Install your system permissions so your flashing software can talk to this device without requiring sudo.
Run this command in your terminal to set up the official ST rules:
bash
sudo wget -O /etc/udev/rules.d/99-stm32dfu.rules https://githubusercontent.com && sudo udevadm control --reload-rules && sudo udevadm trigger

Use code with caution.
What software tool or IDE (e.g., STM32CubeProgrammer, dfu-util, Arduino IDE, or PlatformIO) are you planning to use to flash your code next?
AI responses may include mistakes. Learn more
should i do it every single time or it was one time solving?
You do not have to do the jumper wire trick every single time. It was a one-time troubleshooting step to figure out why your computer was getting confused.
Now that we know exactly how your board behaves, here is how it will work going forward:
1. The Jumper Wire (PA10 to GND)
One-time fix. You can leave this wire out of your breadboard now.
You only need to bring it back if your laptop starts giving you that error -71 connection failure again in the future.
2. The udev Rules Command
One-time fix. Once you run that sudo wget command, your Linux system will permanently remember the permissions for this device. You never have to run it again.
3. The Buttons (BOOT0 and NRST)
Every time you want to upload new code. Microcontrollers can only do one thing at a time. When they are running your code, they cannot listen for new software.
Every time you want to upload a new program, you must use the button sequence (Hold BOOT0 \(\rightarrow \) Tap NRST \(\rightarrow \) Release BOOT0) to force the chip to stop your code and open the DFU gate.
Are you ready to upload your firmware file, and do you need the exact command or tool steps to flash the chip now?


