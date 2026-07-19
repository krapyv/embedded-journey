# BMP280 I2C Driver

A driver that handles both temperature (in 0.01 DegC resolution) and pressure (in Q24.8 format) measurements  via I2C communication protocol. 
Implements its own 12-state machine that coordinates I2C transactions by polling hi2c->state against I2C_STATE_DONE before advancing.
Build on top of the interrupt-driven I2C driver, abstracting raw byte transfers into sensor initialization, calibration reconstruction, and compensated measurement cycles.

## Project Structure

```text
reusable_drivers/
├── devices/bmp280      
│    ├── bmp280.h        # Device handle, state machine enums, register map, and public API signatures
│    └── bmp280.c     # BMP280 peripheral driver: state machine functions, calculation and compensation functions
└── periph/     # Portable peripheral drivers
    ├── i2c.c      # Register-level I2C peripheral driver: EV/ER ISR logic, transaction state machine, and SWRST recovery
    └── i2c.h      # I2C control and state enums, handler struct declaration, extern variable declaration, and function headers
```

## Integration Guide

The user does not need to call individual functions. The user needs to do three steps:

1. Declare and initialize the device handle and measurement configuration:

```c
BMP280_HandleTypeDef hbmp = {
    .hi2c = &hi2c,
    .slave_addr = BMP280_I2C_ADDR,
    .isInitialized = 0
};

BMP280_Ctrl_Meas_t meas = {
    .osrs_p = BMP280_OSRS_P_OVRSMP_1,
    .osrs_t = BMP280_OSRS_T_OVRSMP_1,
    .mode = BMP280_FORCED_MODE
};
```

2. Set state to INIT to start the machine:

```c
    hbmp.state = BMP280_STATE_INIT;
```

3. Call the switch dispatcher every main loop iteration:

```c
while (1) 
{
    switch (hbmp.state)
    {
    case BMP280_STATE_IDLE:
        break;
    case BMP280_STATE_INIT:
        // BMP280_Init(&hbmp, meas)
        break;
    case BMP280_STATE_READ_CALIBRATION:
        break;
    case BMP280_STATE_RECONSTRUCT_CALIBRATION:
        break;
    case BMP280_STATE_CTRL_MEAS:
        break;
    case BMP280_STATE_MEASURING:
        break;
    case BMP280_STATE_READ_MEASUREMENTS:
        break;
    case BMP280_STATE_RECONSTRUCT_MEASUREMENTS:
        break;
    case BMP280_STATE_COMPENSATE:
        break;
    case BMP280_STATE_READY:
        // hbmp.temp_value / 100 -> temperature in DegC
        // hbmp.press_value / 256 / 100 -> pressure in hPa
        break;
    case BMP280_STATE_ERROR:
        break;
    case BMP280_STATE_FAULT:
        break;
    }
}
```

## Low-level Architectural Decisions

### State machine
The state machine has 12 different states:

1. BMP280_STATE_IDLE:
The default state of the system. No transaction is ongoing, nothing happens.

2. BMP280_STATE_INIT:
The state represents the initialization of BMP280 layer (by initialization there is meant requesting the chip ID of the peripheral before any transaction). 
Executes only once per the program lifetime.

3. BMP280_STATE_READ_CALIBRATION:
The state represents the requesting and receiving the calibration parameters.
Executes only once per the program lifetime.

4. BMP280_STATE_RECONSTRUCT_CALIBRATION:
The state showing the process of calibration parameters reconstruction. This step is necessary because each compensation word is a 16-bit signed or unsigned integer value stored in two's complement. As the memory is organized into 8-bit words, two words must always be combined in order to represent the compensation word.
Executes once per the program lifetime.

5. BMP280_STATE_CTRL_MEAS:
The state represents the process of setting the data acquisition options of the device: oversampling of temperature date as well as oversampling of pressure data, and the power mode of the device.
The first state of the second and the next transactions since the implemented main.c uses "forced mode" what means after every transaction the chip goes to sleep and requires a setting of the control data once more.

6. BMP280_STATE_MEASURING:
The state represents the process of barometric measurement.
Polls the status register (0xF3) bit 3 (measuring flag) via repeated I2C reads until the hardware signals measurement completion. Times out after 15ms and transitions to STATE_ERROR.

7. BMP280_STATE_READ_MEASUREMENTS:
The state represents burst reading of raw ADC measurement values from the BMP280.

8. BMP280_STATE_RECONSTRUCT_MEASUREMENTS:
The state represents the process of the raw pressure and temperature measurement output data reconstruction. This step is necessary because the two registers "press" [0xF7..0xF9] and "temp" [0xFA..0xFC] contain three parts of their respective raw output data values: MSB, LSB and XLB.

9. BMP280_STATE_COMPENSATE:
The state used for data calculation: once the calibration values and raw measurement values are received and reconstructed, the function designed for this state calculates compensated temperature and pressure.

10. BMP280_STATE_READY:
The last state of the transaction achieved once the temperature (calculated in resolution 0.01 DegC, for human-readable DegreesC format requires division by 100) and pressure (calculated in Q24.8 format (24 integer bits and 8 fractional bits), for human-readibility in hPa requires subsequent divisions by 256 and 100) values are calculated.

11. BMP280_STATE_ERROR:
The BMP280 experiences an error. In current main.c implementation the state has 3 retries and happy recovery path: once hi2c->state == I2C_STATE_IDLE, increment retries and start the transaction again from scratch.
Once retries >= 3, the hbmp.state is set to STATE_FAULT, since it cannot complete a single transaction.
 
12. BMP280_STATE_FAULT:
The state used for representing the hard fault. Requires power-on reset.

## Known Limitations

### The MEASUREMENTS spelling typo in enum names:

```c
BMP280_STATE_READ_MEASURAMENTS
BMP280_STATE_RECONSTRUCT_MEASURAMENTS
```

### Error recovery in STATE_ERROR:

Now the retry logic skips STATE_INIT and STATE_READ_CALIBRATION, immediately reseting state to STATE_CTRL_MEAS. I have made an assumption that once the calibration data is read and reconstructed, nothing can happen to it. I did not take into consideration a case if the I2C bus experienced a hard fault that corrupted the calibration data already in RAM.
If calibration data in RAM was corrupted, the compensation functions will produce incorrect temperature and pressure values with no indication of the fault.

### Software recovery path from STATE_FAULT

Now there is no software recovery path. The system is permanently dead until reset.
