# Test harness for the project STM32F411 + TXS0108E + MCP2515 + CANable

The test main loop that tests the CAN bus communication between STM32F411 and the personal PC using CANable as the second independent CAN node. 
 Every new incoming data frame is detected by the ISR that sets the flag. The main() handles the CANINTF register clearing and data frames retrieving.
Received frames are inspected via GDB watchpoints on can_int_rx0_header and can_int_rx0_payload. Frame transmission from the PC node is verified via candump on the CANable interface.

## Breadboard circuit

The TXS0108E level shifter is sitting on the breadboard straddling the separation channel. The MCP2515 module sits off-board (male pins, not breadboard-mounted) as well as the CANable USB to CAN debugger-analyzer. 

Rail split: right rails = 3.3V, left rails = 5V. TXS0108E straddles the separation row — VA (3.3V) side faces right rails, VB (5V) side faces left rails.
Level shifter placed directly behind Black Pill to keep A-side hop short per capacitance budget.

TXS0108E side A faces 3.3V power rail, side B - 5V power rail.

MCP2515 VCC is connected to the 5V power rail.
MCP2515 GND is connected to the common ground rail (GND).
MCP2515 SCK is connected to the TXS0108E B1 pin.
MCP2515 SI (MOSI) is connected to the TXS0108E B2 pin.

MCP2515 SO (MISO) is connected directly to STM32 PA6, bypassing the level shifter. STM32 input pins are 5V-tolerant on this line, and MISO is driven by the MCP2515 - the 3.3V STM32 receiver correctly interprets the 5V logic high.

MCP2515 CS is connected to the TXS0108E B3 pin.
MCP2515 INT is connected to the TXS0108E B4 pin.

STM32 PA5 (SCK) is connected to the TXS0108E A1.
STM32 PA7 (MOSI) is connected to the TXS0108E A2.
STM32 PA4 (CS) is connected to the TXS0108E A3.
STM32 PB15 (INT) is connected to the TXS0108E A4.

TXS0108E VA (3.3V side) connected to the 3.3V power rail.
TXS0108E VB (5V side) connected to the 5V power rail.
TXS0108E GND is connected to the common ground rail (GND).
TXS0108E OE pin is connected to 3.3V power rail.

The CANable GND is connected to the common ground rail (GND).
The CANable CAN_H and CAN_L are connected to the MCP2515 CAN_H and CAN_L respectively.

The Normal mode transition error led:
The 220 Ohms resistor terminal 1 is connected to the PA0 of the STM32.
The 220 Ohms resistor terminal 2 is connected to the anode (the longer leg) of the LED.
The cathode (the shorter leg) of the LED is connected to the GND rail.
