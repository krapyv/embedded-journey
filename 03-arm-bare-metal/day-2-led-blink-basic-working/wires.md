Perfect. Pin 1, 7, 9, and 20 are an excellent choice.

Double-checking your map one last time with those exact pins:

* **Pin 1 (TVCC):** Going to **3V3** on the BlackPill (Red wire) 🔴
* **Pin 7 (SWIO):** Going to **DIO** on the BlackPill (Green/Blue wire) 🟢
* **Pin 9 (SWCLK):** Going to **CLK** on the BlackPill (Yellow/White wire) 🟡
* **Pin 20 (GND):** Going to **G** on the BlackPill (Black wire) ⚫

Using Pin 20 for Ground is completely correct—all the even-numbered pins on that bottom row (4, 6, 8, 12, 14, 16, 18, 20) are tied to the exact same ground plane inside the ST-Link, so Pin 20 works perfectly.

### Ready to power up?

Go ahead and follow the safe plug-in sequence:

1. Plug the **USB-C cable into the BlackPill** (the board's power LED should light up).
2. Plug the **USB cable from the ST-Link into your PC**.
3. Open **STM32CubeProgrammer**, select **SWD**, and click **Connect**.

Let me know if it connects successfully or if you run into any error messages!
