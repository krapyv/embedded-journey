# Breadboard circuit

There are three LEDs: PC13 (the Blackpill on-board LED), PA1 (red LED), PA10 (green LED).

1. PC13 blue LED: does not require any settings.

2. PA1 red LED:
- STM32F411 Pin PA1: Connected to a row sharing one terminal of a 220 Ohms Resistor.
- Resistor (Terminal 2): Connected to the Anode (longer leg) of the LED.
- LED Cathode (shorter leg): Connected to a common ground rail (by a black wire) tied directly to the GND pin of the Blackpill.

3. PA10 green LED:
- STM32F411 Pin PA10: Connected to a row sharing one terminal of a 220 Ohms Resistor.
- Resistor (Terminal 2): Connected to the Anode (longer leg) of the LED.
- LED Cathode (shorter leg): Connected to a common ground rail (by a black wire) tied directly to the GND pin of the Blackpill.

4. The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

