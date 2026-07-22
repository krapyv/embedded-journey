#ifndef SPI_H
#define SPI_H
#include "stm32f411.h"

// for the future generalization
// SPI1: NSS1 - PA4, SCK1 - PA5, MISO1 - PA6, MOSI1 - PA7
//       NSS1 - PB15, SCK1 - PB3, MISO1 - PB4, MOSI1 - PB5
// SPI2: NSS2 - PB9 / PB12, SCK2 - PB10 / PB13, MISO2 - PB14, MOSI2 - PB15
// SPI3: NSS3 - PA4 / PA15, SCK3 - PB3 / PB12, MISO3 - PB4, MOSI3 - PB5
// SPI4: NSS4 - PB12, SCK4 - PB14, MISO4 - PA11, MOSI4 - PA1
// SPI5: NSS5 - PB1, SCK5 - PB0, MISO5 - PA12, MOSI5 - PB8 / PA10

typedef enum
{
    SPI_Channel_1 = 0,
    SPI_Channel_2,
    SPI_Channel_3,
    SPI_Channel_4,
    SPI_Channel_5,
} SPI_Channel_t;

typedef struct
{

} SPI_HandleTypeDef;

#endif // SPI_H