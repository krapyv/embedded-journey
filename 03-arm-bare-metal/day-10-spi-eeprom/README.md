## Breadboard circuit

The M95320WT EEPROM is soldered on the SOP-to-DIP adapter and is sitting on the breadboard straddling the separation channel. Pins 1 to 4 sit on the left side, pins 5 to 8 sit on the right side.

Pin 1 - Chip Select (NSS) - is connected to the PA4 row of the STM32F411 via a jumper wire (yellow color).
Pin 2 - MISO - is connected to the PA6 row  of the Blackpill (blue wire).
Pin 3 - Write Protect - is connected to the Vcc power rail (orange wire). Write Protect freezes the size of the area of memory that is protected against Write instruction, should be driven either high or low. To disable write protections, tie to Vcc (3.3V).
Pin 4 - Vss - is connected to the Ground power rail (black wire).
Pin 5 - MOSI - is connected to the PA7 row of the STM32F411 (green wire).
Pin 6 - Serial Clock (SCK/SCLK) - connected to the PA5 row (white wire).
Pin 7 - Hold - connected to the Vcc power rail (orange wire). The Hold signal is used to pause any serial communications with the device without deselecting the device. To disable holds, tie to Vcc (3.3V).
Pin 8 - Vcc - is conntected to the Vcc power rain (red wire).

LEDs:

The "correct" green LED:
The Blackpill's PA0 breadboard row is connected to the row of the 220 Ohms resistor's terminal via a purple wire.
The second resistor's terminal is connected to the row of the LED's longer leg (anode). 
The Vss (Ground) wire is connected to the row of the LED's shorter leg (cathode).

The "error" red LED:
The Blackpill's PB1 breadboard row is connected to the row of the 220 Ohms resistor's terminal via a purple wire.
The second resistor's terminal is connected to the row of the LED's longer leg (anode).
The Vss (Ground) black wire is connected to the row of the LED's shorter leg (cathode).

The EEPROM as well as both LEDs share the same continuous ground rail on the breadboard coming from the STM32F411 GND.


The 220 Ohns resistors:
the resistor only sees the voltage drops across itself, not the LED's forward voltage.

the voltage on the resistor: Vr = Vcc - Vf

on high voltage drop:
Vr = 3.3 - 2.2 = 1.1 V

according to the Ohm's law: I = V / R => I = 1.1 V / 220 Ohms = 0.5 * 10^(-2) = 5 mA

on low voltage drop: 
Vr = 3.3 - 1.8 = 1.5 V

I = approximately 6.8 mA

for modern LEDs with a threshold of up to 20 mA, 5-7 mA is perfect, so the 220 Ohms resistors are the best for this circuit.
