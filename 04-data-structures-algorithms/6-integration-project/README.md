## Breadboard circuit

Two breadboards:
- the first contains the Black Pill and the Motor Driver TB6612FNG (A).
- the second one has the BMP280 and the lever shifter TXS0108E (B).

**Breadboard A:**

One power rail of the breadboard is connected to the 5V pin of the Blackpill (red wire).
The second power rail is connected to the 3.3V.

The TXS0108E level shifter is sitting on the second breadboard straddling the separation channel. The MCP2515 module sits off-board (male pins, not breadboard-mounted) as well as the CANable USB to CAN debugger-analyzer. 

1. Encoder: 

- VCC (5V) is connected to the Blackpill's 5V pin (red wire).
- GND is connected to the Blackpill's GND pin (black wire).
- A is connected to the PA8 (green wire).
- B is connected to the PA9 (blue wire).

Both PA8 and PA9 are 5 V-tolerant.

2. Power side:

- XL4015 OUT+ (red) is connected to the motor driver's VM.
- XL4015 OUT- (black) is connected to the driver's GND (one of three).
- The 470 µF capacitor across VM and GND, right at the driver. The stripe (negative) goes to GND. The long leg is positive.

3. Adapter 230 V AC 12 V DC

IMPORTANT: make sure you limited the XL4015 current to 1 A and 6 V voltage.

The 12 V adapter connects through a DC socket with screw terminals. Check their polarity before wiring:
- Plug the adapter into the DC socket, then into the mains.
- Plug the multimeter on DC volts across the two screw terminals.
- It should read about +12 V with the red probe on the positive terminal. If it reads -12 V, the terminals are swapped. Mark the positive one with a bit of red tape.
- Unplug the adapter from the mains before touching any wiring.

Then wire the socket to the XL4015 input:
- Male pins into the socket's screw terminals.
- Red wire from the socket's `+` to the XL4015's `IN+`, and black from `-` to `IN-`.
- Check with the multimeter's continuity mode, adapter unplugged, that IN+ and IN- aren't shorted and neither are OUT+ and OUT-.

The XL4015 does:
- Steps the voltage down: it's a "step-down" or buck converter, so it takes 12 V in and gives a lower voltage out. You set the output to 6.0 V with the voltage pot, since the motor is rated for 6 V.
- Limits the current: the other pot sets a ceiling of about 1 A. If a bug or a stalled motor tries to draw more, the module holds the current at that value, so the driver doesn't overheat.
- Shows what is happening: the two displays give you the output voltage and the current, so you can see how much the motor draws at each duty.

It is powered from the adapter on its IN terminals, and it feeds the driver from OUT+ and OUT-. The Black Pill is powered separately over USB.

4. Logic side:
- Driver VCC is connected to the Black Pill's 3.3 V (power rail) (red wire).
- Driver GND is connected to the Black Pill's GND (black wire). All grounds are the same node: the XL4015 OUT-, the driver's GND, and the Black Pill GND.
- Driver STBY is connected to the Black Pill's PB14 (yellow wire).
- Driver's AIN1 is connected to the Black Pill's PB12 (white wire).
- Driver's AIN2 is connected to the Black Pill's PB13 (orange wire).
- Driver's PWMA is connected to the Black Pill's PB8 (purple wire).

5. Motor:
- Motor's red wire is connected to the driver's AO1.
- Motor's black wire is connected to the driver's AO2.

This sets which direction is forward: if `speed` comes out negative on the first forward spin, swap these two.

6. The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

**Breadboard B:**

One power rail of the breadboard is connected to the 3.3V of the first breadboard (orange wire). Second power rail is connected to the 5V rail of the first breadboard (orange wire).
Both ground rails of the breadboard B are connected to the ground rails of the breadboard A (brown jumper wires).

7. TXS0108E:

TXS0108E straddles the separation row — VA (3.3V) side faces right rails, VB (5V) side faces left rails.
Level shifter (the second breadboard generally) is placed close enough to the first breadboard to keep A-side hop short per capacitance budget.

TXS0108E side A faces 3.3V power rail, side B - 5V power rail.

MCP2515 VCC is connected to the 5V power rail (red wire).
MCP2515 GND is connected to the common ground rail (GND) (black wire).
MCP2515 SCK is connected to the TXS0108E B1 pin (yellow wire).
MCP2515 SI (MOSI) is connected to the TXS0108E B2 pin (green wire).

MCP2515 SO (MISO) is connected directly to STM32 PA6, bypassing the level shifter. STM32 input pins are 5V-tolerant on this line, and MISO is driven by the MCP2515 - the 3.3V STM32 receiver correctly interprets the 5V logic high.
The wire color - blue.

MCP2515 CS is connected to the TXS0108E B3 pin (white wire).
MCP2515 INT is connected to the TXS0108E B4 pin (gray wire).
MCP2515 J1 should be shorted (brown wire). J1 is 120Ω terminal resistance.

STM32 PA5 (SCK) is connected to the TXS0108E A1 (yellow wire).
STM32 PA7 (MOSI) is connected to the TXS0108E A2 (green wire).
STM32 PA4 (CS) is connected to the TXS0108E A3 (white wire).
STM32 PB15 (INT) is connected to the TXS0108E A4 (gray wire).

TXS0108E VA (3.3V side) connected to the 3.3V power rail (red wire).
TXS0108E VB (5V side) connected to the 5V power rail (red wire).
TXS0108E GND is connected to the common ground rail (GND) (black wire).
TXS0108E OE pin is connected to 3.3V power rail (orange wire).

The CANable GND is connected to the common ground rail (GND) (black wire).
The CANable CAN_H and CAN_L are connected to the MCP2515 CAN_H and CAN_L respectively. The CAN_H wire is yellow, the CAN_L wire is green.
The CANable is connected to the PC's USB port.

8. BMP280:

The BMP280 faces the right side of the power rails (3.3 V).

- VCC is connected to the power rail 3.3V (red jumper wire).
- GND is connected to the Ground rail (black jumper wire).
- CSB is connected to the power rail 3.3V  (orange jumper wire). If the CSB is connected to the VDDIO (VCC), the I2C interface is active. Otherwise, if the CSB is connected to the GND (0V), the SPI interface is active. 
- SDO is connected to the GND rail (brown wire). The driver uses by default address 1110110 (0x76). 
* The driver uses by default SDA1 and SCL1:
- SDA of the BMP280 is connected to the PB7 of the Blackpill (green wire).
- SCL of the BMP280 is connected to the PB6 of the Blackpill (yellow wire).

* I2C is open-drain. The lines need pull-ups to VCC 3.3V to define the idle high state. Without pull-ups configured either externally or internally, the lines float and the bus never reaches a defined idle state.
- In the breadboard circuit there were used 4.7k Ohms pull-up resistors on both SDA and SCL connected to power rails (rise time 217ns from 1000ns available).
