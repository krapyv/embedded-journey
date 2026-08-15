# Test harness for the project I2C + BMP280 Non-blocking measurements

The test main loop that tests the BMP280 non-blocking measurements via I2C using the BMP280 state machine. The successful measurement prints the result over UART on the console and the loop recycles once again. The error state supports 3 retries. Once all retries are exhausted, the state machine transitions permanently to the fault state.

## Breadboard circuit

- VCC is connected to the power rail (red jumper wire).
- GND is connected to the Ground rail (black jumper wire).
- CSB is connected to the power rail  (orange jumper wire). If the CSB is connected to the VDDIO (VCC), the I2C interface is active. Otherwise, if the CSB is connected to the GND (0V), the SPI interface is active. 
- SDO is connected to the GND rail (brown wire). The driver uses by default address 1110110 (0x76). 
* The driver uses by default SDA1 and SCL1:
- SDA of the BMP280 is connected to the PB7 of the Blackpill (green wire).
- SCL of the BMP280 is connected to the PB6 of the Blackpill (yellow wire).

* I2C is open-drain. The lines need pull-ups to VCC to define the idle high state. Without pull-ups configured either externally or internally, the lines float and the bus never reaches a defined idle state.
- In the breadboard circuit there were used 4.7k Ohms pull-up resistors on both SDA and SCL connected to power rails (rise time 217ns from 1000ns available).

* The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.
