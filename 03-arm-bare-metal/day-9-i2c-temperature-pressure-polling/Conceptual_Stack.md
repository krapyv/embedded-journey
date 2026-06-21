The conceptual stack from raw silicon to compensated output:

* Hardware layer: open-drain GPIO, AF4, pull-up budget, RC timing
* Peripheral init: RCC clock gating, CR2/CCR/TRISE/CR1 sequence, PE last
* Transaction layer: START/ADDR/REPEATED START/NACK/STOP mechanics, SB/ADDR/TxE/BTF/RxNE flag sequences, AF and BUSY error detection, SWRST recovery
* Sensor layer: chip ID verification, ctrl_meas/config register writes, burst read of 0x88-0x9F calibration + 0xF7-0xFC raw data
* Compensation layer: t_fine ordering, fixed-point int32_t math, 0.01°C scaling

The progression after polling would naturally be:

* Polling → works, but CPU is blocked during every I2C transaction
* Interrupt-driven → CPU is free during the transaction, but you need a state machine to track where in the sequence the ISR is
* DMA → CPU is completely uninvolved until the buffer is full, but now you have cache coherency and buffer ownership concerns
