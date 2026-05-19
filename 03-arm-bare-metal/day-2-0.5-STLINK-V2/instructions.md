The Absolute Visual Blueprint
Look at the flat grey ribbon cable coming out of your ST-LINK box. Lay the connector flat on your table so that the plastic alignment notch (the little bump) is pointing straight down into the table (hidden underneath).

With the notch pointing DOWN into the table, look directly at the open holes where the wires plug in:

The row closest to you (Bottom Row): These are all the ODD numbers (1,3,5,7...19).

The row furthest from you (Top Row): These are all the EVEN numbers (2,4,6,8...20).

The RED STRIPE edge: This is on your LEFT side now.

The PLAIN GREY edge: This is on your RIGHT side now.

Step-by-Step Re-Wiring (With Notch Facing DOWN)
Keep the USB cables completely unplugged. Pull all 4 wires out of the ST-LINK connector and place them exactly into these holes using this blueprint:

The TVCC (Power Sense) Wire: Plug this into the Bottom-Left corner hole (This is Pin 1, right on the red stripe edge, closest row to you). Run this to the BlackPill's 3V3 pin.

The GND (Ground) Wire: Plug this into the Top-Right corner hole (This is Pin 20, right on the plain grey edge, furthest row from you). Run this to the BlackPill's GND pin.

The SWDIO Data Wire: Look at the bottom row (closest to you), start at the red stripe on the left, and count 4 holes over. Plug it into Pin 7. Run this to BlackPill DIO.

The SWCLK Data Wire: Look at the bottom row (closest to you), start at the red stripe on the left, and count 5 holes over. Plug it into Pin 9. Run this to BlackPill CLK.

Try the Test Again
Once you have matched this "notch down" layout:

Plug only the BlackPill's USB-C cable into the computer.

Does the red PWR LED on the BlackPill finally light up brightly?
